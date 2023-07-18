// Copyright (c) 2011 rubicon IT GmbH
#include <stdio.h>
#include <string.h>
//#include <malloc.h>
#include <math.h>
#include <errno.h>
#include "GLFormula.h"
#include "Helper/MathTools.h"
#include "Helper/StringHelper.h"
#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#endif

#if defined(SYNRAD)
#include "../src/SynRad.h"
#endif

#if defined(MOLFLOW)
extern MolFlow* mApp;
#endif

#if defined(SYNRAD)
extern SynRad* mApp;
#endif

const std::map<std::string, OperandType> GLFormula::mathExpressionsMap = {
	{"abs(", OperandType::ABS},
	{"abs(", OperandType::ABS},
	{"asin(", OperandType::ASIN},
	{"acos(", OperandType::ACOS},
	{"atan(", OperandType::ATAN},

	{"cos(", OperandType::COS},
	{"cosh(", OperandType::COSH},

	{"exp(", OperandType::EXP},

	{"fact(", OperandType::FACT},

	{"ln(", OperandType::LN},
	{"log2(", OperandType::LOG2},
	{"log10(", OperandType::LOG10},

	{"sin(", OperandType::SIN},
	{"sqrt(", OperandType::SQRT},
	{"sinh(", OperandType::SINH},

	{"tan(", OperandType::TAN},
	{"tanh(", OperandType::TANH}
};

// GLFormula

void GLFormula::SetName(const std::string& _name) {
	this->name = _name;
}

std::string GLFormula::GetName() {
	return name;
}

void GLFormula::SetExpression(const std::string& _expression) {
	this->expression = _expression;
}

std::string GLFormula::GetExpression() {
	return expression;
}

// Return fisrt significative char in the string to analyze, repeat 'times' times 
void GLFormula::AV(size_t times)
{
	for (size_t i = 0; i < times; i++) {
		do {
			++currentPos;
			if (currentPos >= expression.size()) {
				currentChar = 0; //imitate C string
				//throw Error("Formula parser reached end of formula expecting more characters");
			}
			else {
				currentChar = expression[currentPos];
			}
		} while (currentChar == ' ' || currentChar == '\t');
	}
}

// Set global error
void GLFormula::SetParseError(const std::string& _errMsg, int pos) {
	this->parseErrorMsg = fmt::format("{} at pos. {}", _errMsg, pos + 1);
	this->hasParseError = true;
}

std::list<Variable>::iterator GLFormula::FindVar(const std::string& var_name) {
	auto p = variables.begin();
	while (p != variables.end() && !iequals(p->varName, var_name)) {
		p++;
	}
	return p; //either points to found element or variables.end()
}

// Add variables into the chained list
std::list<Variable>::iterator GLFormula::AddVar(const std::string& _var_name)
{
	// Search if already added
	auto pos = FindVar(_var_name);
	if (pos == variables.end()) { //not found
		Variable newVar;
		newVar.varName = _var_name;
		variables.push_front(newVar); //insert to front
		return variables.begin(); //return iterator to front
	}
	else {
		return pos; //found position
	}
}

// Add node into the evaluation tree
// Sets "node" pointer to a newly created element with type,value passed as arguments, and left/right set
std::unique_ptr<EvalTreeNode> GLFormula::AddNode(OperandType type, std::variant<std::monostate, double, std::list<Variable>::iterator> value, std::unique_ptr<EvalTreeNode> left, std::unique_ptr<EvalTreeNode> right) {
	EvalTreeNode newNode(value);
	newNode.type = type;
	//Transfer ownership to the new node:
	newNode.left = std::move(left);
	newNode.right = std::move(right);
	return std::make_unique<EvalTreeNode>(newNode);
}

// Grammar functions

double GLFormula::ReadDouble()
{
	std::string numberStr, exponentStr;
	int negativeExponent;

	do {
		numberStr += currentChar;
		AV();
	} while ((currentChar >= '0' && currentChar <= '9') || currentChar == '.'); //digit

	if (currentChar == 'E' || currentChar == 'e') {
		AV();
		negativeExponent = false;

		if (currentChar == '-') {
			negativeExponent = true;
			AV();
		}

		while (currentChar >= '0' && currentChar <= '9') //exponent digit
		{
			exponentStr += currentChar;
			AV();
		}
		try {
			auto dummy = std::stoi(exponentStr); //test if exponent is parsable
		}
		catch (...) {
			throw Error("Incorrect exponent \""+exponentStr);
		}
		if (negativeExponent) { numberStr = numberStr + "e-" + exponentStr; }
		else { numberStr = numberStr + "e" + exponentStr; }
	}
	try {
		return std::stod(numberStr);
	}
	catch (...) {
		throw Error("Incorrect number \"" + numberStr);
	}
}

std::string GLFormula::ReadVariable() {
	int localPos;
	localPos = currentPos;

	std::string result;
	do {
		result += currentChar;
		AV();
		if (result.length() > 64) SetParseError("Variable name too long (max.64)", localPos);
	} while ((!hasParseError) && (
		(currentChar >= 'A' && currentChar <= 'Z') ||
		(currentChar >= 'a' && currentChar <= 'z') ||
		(currentChar >= '0' && currentChar <= '9') ||
		(currentChar == '_') ||
		(currentChar == '[') ||
		(currentChar == ']')));
	return result;
}

std::optional< std::unique_ptr<EvalTreeNode> > GLFormula::CheckAndTreatTerm(const std::string& term, OperandType operand) { //searches for "term" in expression, and makes tree node with operand. returns true if found and treated
	//Check if the expression's part from "currentPos" starts with "term", if yes, treat it by creating a new node
	//It is not an error if not, then other terms will be checked
	//However, if there is no closing ) then it's a parsing error
	if (iBeginsWith(expression.substr(currentPos), term)) {
		AV(term.length());
		auto left = ReadExpression();
		auto resultNode = AddNode(operand, std::monostate{}, std::move(left), nullptr);
		if (currentChar != ')') {
			SetParseError(") expected", currentPos);
		}
		else AV();
		return resultNode;
	}
	else return std::nullopt;
}

std::unique_ptr<EvalTreeNode> GLFormula::ReadTerm() //read mathematical term at current pos, write to node
{
	std::unique_ptr<EvalTreeNode> resultNode = nullptr;
	if (hasParseError) return resultNode;

	if (currentChar == '.' || (currentChar >= '0' && currentChar <= '9')) { //digit
		try {
			double val = ReadDouble();
			resultNode = AddNode(OperandType::TDOUBLE, val, nullptr, nullptr);
		}
		catch (std::exception error) {
			SetParseError(error.what(), currentPos);
		}
	}
	else if (currentChar == '(') {

		AV(); //opening (
		resultNode = ReadExpression();
		if (currentChar != ')') {
			SetParseError(") expected", currentPos);
		}
		AV(); //closing )
	}
	else if (currentChar == '"') { //support for variables referring to "formula name"
		std::string formulaName = "\"";AV(); //opening "
		while (currentChar != '\0' && currentChar != '"') {
			formulaName += currentChar;
			AV();
		}
		if (currentChar == '\0') {
			SetParseError("closing \" expected", currentPos);
		}
		else if (formulaName.length() == 1) {
			SetParseError("Empty formula name \"\" not valid", currentPos);
		}
		else {
			formulaName += currentChar;AV(); //closing "
			auto varIterator = AddVar(formulaName);
			resultNode = AddNode(OperandType::TVARIABLE, varIterator, nullptr, nullptr);
		}
	}
	else if (currentChar == '-') {
		// unary operator
		AV();
		auto left = ReadTerm();
		resultNode = AddNode(OperandType::MINUS1, std::monostate{}, std::move(left), nullptr);
	}
	else if (std::string term = "AVG("; iBeginsWith(expression.substr(currentPos), term)) {
		std::string avgExpression;
		avgExpression += currentChar;
		while (currentChar != ')') {
			if (currentChar == 0) {
				SetParseError("AVG( expression without closing )", currentPos);
				break;
			}
			else {
				AV();
				avgExpression += currentChar;
			}
		};
		if (!hasParseError) {
			AV();
			auto varIterator = AddVar(avgExpression);
			resultNode = AddNode(OperandType::TVARIABLE, varIterator, nullptr, nullptr);
		}
	}
	else if (iBeginsWith(expression,"SUM(")) {
		std::string sumExpression;
		sumExpression += currentChar;
		while (currentChar != ')') {
			if (currentChar == 0) {
				SetParseError("SUM( expression without closing )", currentPos);
				break;
			} else {
				AV();
				sumExpression += currentChar;
			}
		};
		if (!hasParseError) {
			AV();
			auto varIterator = AddVar(sumExpression);
			resultNode = AddNode(OperandType::TVARIABLE, varIterator, nullptr, nullptr);
		}
	}
	else if (std::string term = "pi"; iBeginsWith(expression.substr(currentPos), term)) {
		AV(term.length());
		resultNode = AddNode(OperandType::TDOUBLE, M_PI, nullptr, nullptr);
	}
	else if (std::string term = "pow("; iBeginsWith(expression.substr(currentPos), term)) {
		AV(term.length());
		auto left = ReadExpression();
		if (currentChar != ',') SetParseError(", expected", currentPos);
		AV();
		auto right = ReadExpression();
		resultNode = AddNode(OperandType::POW, std::monostate{}, std::move(left), std::move(right));
		if (currentChar != ')') SetParseError(") expected", currentPos);
		AV();
	}
	else {

		// Math functions
		for (const auto& key : mathExpressionsMap) {
			auto res = CheckAndTreatTerm(key.first, key.second);
			if (res.has_value()) {
				resultNode = std::move(res.value());
				return resultNode;
			}

		}

		//Variable name
		if ((currentChar >= 'A' && currentChar <= 'Z') ||
			(currentChar >= 'a' && currentChar <= 'z') ||
			(currentChar == '_')) {
			auto varIterator = AddVar(ReadVariable());
			resultNode = AddNode(OperandType::TVARIABLE, varIterator, nullptr, nullptr);
		}
		else {
			//Nothing could treat the term, set error	
			SetParseError("Syntax error", currentPos);
		}
	}
	return resultNode;
}

std::unique_ptr<EvalTreeNode> GLFormula::ReadPower() //parse "(left)^(right)"
{
	std::unique_ptr<EvalTreeNode> resultNode = nullptr;
	if (hasParseError) return resultNode;

	auto left = ReadTerm();
	if (currentChar == '^')
	{
		AV();
		auto right = ReadTerm();
		resultNode = AddNode(OperandType::POWER, std::monostate{}, std::move(left), std::move(right));
	}
	else {
		resultNode = std::move(left);
	}
	return resultNode;
}

std::unique_ptr<EvalTreeNode> GLFormula::ReadFactor()
{
	if (hasParseError) return nullptr;

	auto left = ReadPower();

	while ((currentChar == '*' || currentChar == '/') && !hasParseError) {
		auto opType = currentChar == '*' ? OperandType::MUL : OperandType::DIV;
		AV();
		auto right = ReadPower();
		left = AddNode(opType, std::monostate{}, std::move(left), std::move(right));
		break;
	}
	return left;
}

std::unique_ptr<EvalTreeNode> GLFormula::ReadExpression()
{
	if (hasParseError) return nullptr;

	auto left = ReadFactor();

	while ((currentChar == '+' || currentChar == '-') && !hasParseError)
	{
		auto opType = currentChar == '+' ? OperandType::PLUS : OperandType::MINUS;
		AV();
		auto right = ReadFactor();
		left = AddNode(opType, std::monostate{}, std::move(left), std::move(right));
	}
	return left;
}

std::string GLFormula::GetParseErrorMsg() {
	return parseErrorMsg;
}

bool GLFormula::Parse()
{
	if (expression.empty()) {
		SetParseError("Empty expression",0);
		return false;
	}

	currentPos = 0;
	currentChar = expression[0];
	
	hasParseError = false;
	parseErrorMsg = "";

	evalTree.reset();
	variables.clear();

	try {
		evalTree = ReadExpression();
	}
	catch (std::exception& err) {
		SetParseError(err.what(), currentPos);
	}

	if (currentPos != (int)expression.length()) { //couldn't parse till end
		SetParseError("Syntax error", currentPos);
	}

	if (hasParseError) {
		evalTree.reset();
		variables.clear();
	}

	return !hasParseError;
}

double factorial(double x) {

	int f = (int)(x + 0.5);
	size_t r = 1;
	for (int i = 1; i <= f; i++) {
		r = (size_t)i * r;
	}
	return (double)r;

}

double GLFormula::EvaluateNode(const std::unique_ptr<EvalTreeNode>& node) {

	double a, b; //eval. of left and right
	if (node->left) {
		a = EvaluateNode(node->left);
	}
	if (node->right) {
		b = EvaluateNode(node->right);
	}
	double r; //temp result before error check

	switch (node->type) {
	case OperandType::PLUS:
		return a + b;
	case OperandType::MINUS:
		return a - b;
	case OperandType::MUL:
		return a * b;
	case OperandType::DIV:
		if (b == 0.0) {
			throw Error("Division by 0");
		}
		else return a / b;
	case OperandType::POW:
		r = pow(a, b);
		if (errno != 0) {
			throw Error(strerror(errno));
		}
		else return r;
	case OperandType::COS:
		r = cos(a);
		if (errno != 0) {
			throw Error(strerror(errno));
		}
		else return r;
	case OperandType::CI95:
		return 1.96 * sqrt(a * (1.0 - a) / b);
	case OperandType::FACT:
		r = factorial(a);
		if (errno != 0) {
			throw Error(strerror(errno));
		}
		else return r;
	case OperandType::ACOS:
		r = acos(a);
		if (errno != 0) {
			throw Error(strerror(errno));
		}
		else return r;
	case OperandType::SIN:
		r = sin(a);
		if (errno != 0) {
			throw Error(strerror(errno));
		}
		else return r;
	case OperandType::ASIN:
		r = asin(a);
		if (errno != 0) {
			throw Error(strerror(errno));
		}
		else return r;
	case OperandType::COSH:
		r = cosh(a);
		if (errno != 0) {
			throw Error(strerror(errno));
		}
		else return r;
	case OperandType::SINH:
		r = sinh(a);
		if (errno != 0) {
			throw Error(strerror(errno));
		}
		else return r;
	case OperandType::EXP:
		r = exp(a);
		if (errno != 0) {
			throw Error(strerror(errno));
		}
		else return r;
	case OperandType::LN:
		r = log(a);
		if (errno != 0) {
			throw Error(strerror(errno));
		}
		else return r;
	case OperandType::LOG10:
		r = log10(a);
		if (errno != 0) {
			throw Error(strerror(errno));
		}
		else return r;
	case OperandType::LOG2:
		r = log(a) / log(2.0);
		if (errno != 0) {
			throw Error(strerror(errno));
		}
		else return r;
	case OperandType::SQRT:
		r = sqrt(a);
		if (errno != 0) {
			throw Error(strerror(errno));
		}
		else return r;
	case OperandType::TAN:
		r = tan(a);
		if (errno != 0) {
			throw Error(strerror(errno));
		}
		else return r;
	case OperandType::ATAN:
		r = atan(a);
		if (errno != 0) {
			throw Error(strerror(errno));
		}
		else return r;
	case OperandType::TANH:
		r = tanh(a);
		if (errno != 0) {
			throw Error(strerror(errno));
		}
		else return r;
	case OperandType::ABS:
		return std::abs(a);
	case OperandType::MINUS1:
		return -a;
	case OperandType::TDOUBLE:
		return std::get<double>(node->value);
	case OperandType::TVARIABLE:
		return std::get<std::list<Variable>::iterator>(node->value)->value;
	}
	throw Error("Unknown operand type");
}

size_t GLFormula::GetNbVariable() {
	return variables.size();
}

std::list<Variable>::iterator GLFormula::GetVariableAt(size_t n) {
	auto it = variables.begin();
	std::advance(it, std::min(n, variables.size() - 1));
	return it;
}

void GLFormula::SetVariableValue(const std::string& name, double value) {
	auto v = FindVar(name);
	if (v != variables.end()) v->value = value;
}

std::string GLFormula::GetEvalErrorMsg()
{
	return evalErrorMsg;
}

void GLFormula::SetEvalError(const std::string& errMsg)
{
	evalErrorMsg = errMsg;
	hasEvalError = true;
}

int GLFormula::GetCurrentPos() {
	return currentPos;
}

double GLFormula::Evaluate()
{	
	if (!evalTree) throw Error("Formula not parsed");
	if (hasParseError) throw Error("Formula couldn't be parsed");
	if (hasEvalError) throw Error(evalErrorMsg); //created at variable evaluation during EvaluateFormulaVariables()->EvaluateVariable()

	hasEvalError = false;
	errno = 0;
		
	try {
		return EvaluateNode(evalTree);
	}
	catch (std::exception& err) {
		SetEvalError(err.what());
		throw err;
	}
}