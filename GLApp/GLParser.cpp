// Copyright (c) 2011 rubicon IT GmbH
#include <stdio.h>
#include <string.h>
//#include <malloc.h>
#include <math.h>
#include <errno.h>
#include "GLParser.h"
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

// Extract a part of the string to analyze 
std::string GLFormula::Extract(int n)
{
	return expression.substr(0, n);
}

// Return fisrt significative char in the string to analyze, repeat 'times' times 
void GLFormula::AV(size_t times)
{
	for (size_t i = 0; i < times; i++) {
		do {
			currentPos++;
			currentChar = expression[currentPos];
		} while (currentChar == ' ' || currentChar == '\t');
	}
}

// Set global error
void GLFormula::SetError(const std::string& _errMsg, int pos) {
	this->errMsg = fmt::format("{} at pos. {}", _errMsg, pos + 1);
	this->error = true;
}

std::list<Variable>::iterator GLFormula::FindVar(const std::string& var_name) {
	auto p = varList.begin();
	while (p != varList.end() && !iequals(p->varName, var_name)) {
		p++;
	}
	return p; //either points to found element or varList.end()
}

// Add variables into the chained list
std::list<Variable>::iterator GLFormula::AddVar(const std::string& _var_name)
{
	// Search if already added
	auto pos = FindVar(_var_name);
	if (pos == varList.end()) { //not found
		Variable newVar;
		newVar.varName = _var_name;
		varList.push_front(newVar); //insert to front
		return varList.begin(); //return iterator to front
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

std::optional<double> GLFormula::ReadDouble()
{
	std::string numberStr, exponentStr;
	int  localPos;
	int negativeExponent;

	localPos = currentPos;

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
			SetError("Incorrect exponent in number", localPos);
			return std::nullopt;
		}
		if (negativeExponent) { numberStr = numberStr + "e-" + exponentStr; }
		else { numberStr = numberStr + "e" + exponentStr; }
	}
	try {
		return std::stod(numberStr);
	}
	catch (...) {
		SetError("Incorrect number", localPos);
		return std::nullopt;
	}
}

std::string GLFormula::ReadVariable() {
	int localPos;
	localPos = currentPos;

	std::string result;
	do {
		result += currentChar;
		AV();
		if (result.length() > 64) SetError("Variable name too long (max.64)", localPos);
	} while ((!error) && (
		(currentChar >= 'A' && currentChar <= 'Z') ||
		(currentChar >= 'a' && currentChar <= 'z') ||
		(currentChar >= '0' && currentChar <= '9') ||
		(currentChar == '_') ||
		(currentChar == '[') ||
		(currentChar == ']')));
	return result;
}

std::optional< std::unique_ptr<EvalTreeNode> > GLFormula::TreatTerm(const std::string& term, OperandType operand) { //searches for "term" in expression, and makes tree node with operand. returns true if found and treated
	if (iBeginsWith(expression.substr(currentPos), term)) {
		AV(term.length());
		auto left = ReadExpression();
		auto resultNode = AddNode(operand, std::monostate{}, std::move(left), nullptr);
		if (currentChar != ')') {
			SetError(") expected", currentPos);
		}
		else AV();
		return resultNode;
	}
	else return std::nullopt;
}

std::unique_ptr<EvalTreeNode> GLFormula::ReadTerm() //read mathematical term at current pos, write to node
{
	std::unique_ptr<EvalTreeNode> resultNode = nullptr;
	if (error) return resultNode;

	if (currentChar == '.' || (currentChar >= '0' && currentChar <= '9')) { //digit
		auto val = ReadDouble();
		if (val.has_value() && !error) resultNode = AddNode(OperandType::TDOUBLE, val.value(), nullptr, nullptr);
	}
	else if (currentChar == '(') {

		AV(); //opening (
		resultNode = ReadExpression();
		if (currentChar != ')') {
			SetError(") expected", currentPos);
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
			SetError("closing \" expected", currentPos);
		}
		else if (formulaName.length() == 1) {
			SetError("Empty formula name \"\" not valid", currentPos);
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
			AV();
			avgExpression += currentChar;
		};
		AV();
		auto varIterator = AddVar(avgExpression);
		resultNode = AddNode(OperandType::TVARIABLE, varIterator, nullptr, nullptr);
	}
	else if (iequals(Extract(4), "SUM(")) {
		std::string sumExpression;
		sumExpression += currentChar;
		while (currentChar != ')') {
			AV();
			sumExpression += currentChar;
		};
		AV();
		auto varIterator = AddVar(sumExpression);
		resultNode = AddNode(OperandType::TVARIABLE, varIterator, nullptr, nullptr);
	}
	else if (std::string term = "pi"; iBeginsWith(expression.substr(currentPos), term)) {
		AV(term.length());
		resultNode = AddNode(OperandType::TDOUBLE, M_PI, nullptr, nullptr);
	}
	else if (std::string term = "pow("; iBeginsWith(expression.substr(currentPos), term)) {
		AV(term.length());
		auto left = ReadExpression();
		if (currentChar != ',') SetError(", expected", currentPos);
		AV();
		auto right = ReadExpression();
		resultNode = AddNode(OperandType::POW, std::monostate{}, std::move(left), std::move(right));
		if (currentChar != ')') SetError(") expected", currentPos);
		AV();
	}
	else {

		// Math functions
		for (const auto& key : mathExpressionsMap) {
			auto res = TreatTerm(key.first, key.second);
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
			SetError("Syntax error", currentPos);
		}
	}
	return resultNode;
}

std::unique_ptr<EvalTreeNode> GLFormula::ReadPower() //parse "(left)^(right)"
{
	std::unique_ptr<EvalTreeNode> resultNode = nullptr;
	if (error) return resultNode;

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
	if (error) return nullptr;

	auto left = ReadPower();

	while ((currentChar == '*' || currentChar == '/') && !error) {
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
	if (error) return nullptr;

	auto left = ReadFactor();

	while ((currentChar == '+' || currentChar == '-') && !error)
	{
		auto opType = currentChar == '+' ? OperandType::PLUS : OperandType::MINUS;
		AV();
		auto right = ReadFactor();
		left = AddNode(opType, std::monostate{}, std::move(left), std::move(right));
	}
	return left;
}

std::string GLFormula::GetErrorMsg() {
	return errMsg;
}

bool GLFormula::Parse()
{
	if (expression.empty()) {
		errMsg = "Empty expression";
		return false;
	}

	currentPos = 0;
	currentChar = expression[0];
	error = false;
	errMsg = "No error";

	evalTree.reset();
	varList.clear();

	evalTree = ReadExpression();

	if (currentPos != (int)expression.length()) { //couldn't parse till end
		SetError("Syntax error", currentPos);
	}

	if (error) {
		evalTree.reset();
		varList.clear();
	}

	return !error;
}

double factorial(double x) {

	int f = (int)(x + 0.5);
	size_t r = 1;
	for (int i = 1; i <= f; i++) {
		r = (size_t)i * r;
	}
	return (double)r;

}

std::optional<double> GLFormula::EvaluateNode(const std::unique_ptr<EvalTreeNode>& node) {

	double a, b; //eval. of left and right
	if (node->left) {
		auto res = EvaluateNode(node->left);
		if (!res.has_value()) return std::nullopt;
		else a = res.value();
	}
	if (node->right) {
		auto res = EvaluateNode(node->right);
		if (!res.has_value()) return std::nullopt;
		else b = res.value();
	}
	double r; //temp result before error check

	if (error) return std::nullopt;

	switch (node->type) {
	case OperandType::PLUS:
		return a + b;
	case OperandType::MINUS:
		return a - b;
	case OperandType::MUL:
		return a * b;
	case OperandType::DIV:
		if (b == 0.0) {
			error = true;
			errMsg = "Divide by 0";
			return std::nullopt;
		}
		else return a / b;
	case OperandType::POW:
		r = pow(a, b);
		if (errno != 0) {
			error = true;
			errMsg = strerror(errno);
			return std::nullopt;
		}
		else return r;
	case OperandType::COS:
		r = cos(a);
		if (errno != 0) {
			error = true;
			errMsg = strerror(errno);
			return std::nullopt;
		}
		else return r;
	case OperandType::CI95:
		return 1.96 * sqrt(a * (1.0 - a) / b);
	case OperandType::FACT:
		r = factorial(a);
		if (errno != 0) {
			error = true;
			errMsg = strerror(errno);
			return std::nullopt;
		}
		else return r;
	case OperandType::ACOS:
		r = acos(a);
		if (errno != 0) {
			error = true;
			errMsg = strerror(errno);
			return std::nullopt;
		}
		else return r;
	case OperandType::SIN:
		r = sin(a);
		if (errno != 0) {
			error = true;
			errMsg = strerror(errno);
			return std::nullopt;
		}
		else return r;
	case OperandType::ASIN:
		r = asin(a);
		if (errno != 0) {
			error = true;
			errMsg = strerror(errno);
			return std::nullopt;
		}
		else return r;
	case OperandType::COSH:
		r = cosh(a);
		if (errno != 0) {
			error = true;
			errMsg = strerror(errno);
			return std::nullopt;
		}
		else return r;
	case OperandType::SINH:
		r = sinh(a);
		if (errno != 0) {
			error = true;
			errMsg = strerror(errno);
			return std::nullopt;
		}
		else return r;
	case OperandType::EXP:
		r = exp(a);
		if (errno != 0) {
			error = true;
			errMsg = strerror(errno);
			return std::nullopt;
		}
		else return r;
	case OperandType::LN:
		r = log(a);
		if (errno != 0) {
			error = true;
			errMsg = strerror(errno);
			return std::nullopt;
		}
		else return r;
	case OperandType::LOG10:
		r = log10(a);
		if (errno != 0) {
			error = true;
			errMsg = strerror(errno);
			return std::nullopt;
		}
		else return r;
	case OperandType::LOG2:
		r = log(a) / log(2.0);
		if (errno != 0) {
			error = true;
			errMsg = strerror(errno);
			return std::nullopt;
		}
		else return r;
	case OperandType::SQRT:
		r = sqrt(a);
		if (errno != 0) {
			error = true;
			errMsg = strerror(errno);
			return std::nullopt;
		}
		else return r;
	case OperandType::TAN:
		r = tan(a);
		if (errno != 0) {
			error = true;
			errMsg = strerror(errno);
			return std::nullopt;
		}
		else return r;
	case OperandType::ATAN:
		r = atan(a);
		if (errno != 0) {
			error = true;
			errMsg = strerror(errno);
			return std::nullopt;
		}
		else return r;
	case OperandType::TANH:
		r = tanh(a);
		if (errno != 0) {
			error = true;
			errMsg = strerror(errno);
			return std::nullopt;
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
	return std::nullopt;
}

size_t GLFormula::GetNbVariable() {
	return varList.size();
}

std::list<Variable>::iterator GLFormula::GetVariableAt(size_t n) {
	auto it = varList.begin();
	std::advance(it, std::min(n, varList.size() - 1));
	return it;
}

void GLFormula::SetVariable(const std::string& name, double value) {
	auto v = FindVar(name);
	if (v != varList.end()) v->value = value;
}

std::string GLFormula::GetVariableEvalError()
{
	return variableEvalErrorMsg;
}

void GLFormula::SetVariableEvalError(const std::string& errMsg)
{
	variableEvalErrorMsg = errMsg;
	hasVariableEvalError = true;
}

int GLFormula::GetCurrentPos() {
	return currentPos;
}

std::optional<double> GLFormula::Evaluate()
{
	error = false;
	errno = 0;

	/* Evaluate expression */

	if (evalTree)
	{
		auto res = EvaluateNode(evalTree);
		if (res.has_value()) return res.value();
	}

	//Here either evalTree is nullptr or EvaluateNode returned nullopt
	error = true;
	return std::nullopt;
}