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
extern MolFlow *mApp;
#endif

#if defined(SYNRAD)
extern SynRad *mApp;
#endif

// GLFormula

void GLFormula::SetName(const std::string& _name) {
  this->name=_name;
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
  return expression.substr(0,n);
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
void GLFormula::SetError(const std::string& _errMsg,int pos) {
  this->errMsg=fmt::format("{} at pos. {}", _errMsg, pos);
  this->error=true;
}

std::list<Variable>::iterator GLFormula::FindVar(const std::string& var_name) {
    auto p = varList.begin();
    while(!iequals(p->varName, var_name)) {
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
void GLFormula::AddNode(int type, std::variant<std::monostate, double, std::list<Variable>::iterator> value, std::shared_ptr<EtreeNode> node, std::shared_ptr<EtreeNode> left, std::shared_ptr<EtreeNode> right) {

	node = std::make_shared<EtreeNode>(value);
    node->type = type;
	node->left = left;
	node->right = right;
	
}

// Grammar functions

void GLFormula::ReadDouble(double *R)
{
  std::string S,ex;
  int  c;
  int  p;
  int nega;
  int n;

  p=currentPos;

  do {
    S+= currentChar;
    AV();
  }while ( (currentChar>='0' && currentChar<='9') || currentChar=='.');

  if (currentChar=='E' || currentChar=='e') {
    AV();
    nega=false;

    if (currentChar=='-') {
      nega=true;
      AV();
    }

    while (currentChar>='0' && currentChar<='9')
    {
      ex+=currentChar;
      AV();
    }
    try {
        n = std::stoi(ex);
    } catch (...) {
        SetError("Incorrect exponent in number",p);
        return;
    }
	if (nega) { S = S + "e-" + ex; }
	else { S = S + "e" + ex; }
  }
  try {
      *R = std::stod(S);
  }
  catch (...) {
      SetError("Incorrect number", p);
  }
}

std::string GLFormula::ReadVariable() {
  int i=0;
  int p;
  p=currentPos;

  std::string result;
  do {
    result +=currentChar;
    AV();
    i++;
    if(i>=63) SetError("Variable name too long",p);
  } while ( (!error) && (
     (currentChar>='A' && currentChar<='Z') || 
     (currentChar>='a' && currentChar<='z') ||
     (currentChar>='0' && currentChar<='9') ||
     (currentChar=='_') ||
	 (currentChar=='[') ||
	 (currentChar==']')));
  return result;
}

bool GLFormula::TreatTerm(const std::string& term, int operand, std::shared_ptr<EtreeNode> node) { //searches for "term" in expression, and makes tree node with operand. returns true if found and treated
    if (std::string term = "abs("; iBeginsWith(expression, term)) {
        AV(term.length());
        std::shared_ptr<EtreeNode> left;
        ReadExpression(left);
        AddNode(OPER_ABS, std::monostate{}, node, left, nullptr);
        if (currentChar != ')') SetError(") expected", currentPos);
        AV();
        return true;
    }
    else return false;
}

void GLFormula::ReadTerm(std::shared_ptr<EtreeNode> node) //read mathematical term at current pos, write to node
{

	if (error) return;

	if (currentChar == '.') {
		double val;
		ReadDouble(&val);
		if (!error) AddNode(TDOUBLE, val, node, nullptr, nullptr);
	}
	else if (currentChar == '(') {

		AV(); //opening (
		ReadExpression(node);
		if (currentChar != ')') {
			SetError(") expected", currentPos);
		}
		AV(); //closing )
	}
	else if (currentChar == '-') {
		// unary operator
		AV();
		std::shared_ptr<EtreeNode> left; ReadTerm(left);
		AddNode(OPER_MINUS1, std::monostate{}, node, left, nullptr);
	}
    
    // Math functions
    if (TreatTerm("abs(", OPER_ABS, node)) return;
    if (TreatTerm("asin(", OPER_ASIN, node)) return;
    if (TreatTerm("acos(", OPER_ACOS, node)) return;
    if (TreatTerm("atan(", OPER_ATAN, node)) return;

    if (TreatTerm("cos(", OPER_COS, node)) return;
    if (TreatTerm("cosh(", OPER_COSH, node)) return;

    if (TreatTerm("exp(", OPER_EXP, node)) return;

    if (TreatTerm("fact(", OPER_FACT, node)) return;

    if (TreatTerm("ln(", OPER_LN, node)) return;
    if (TreatTerm("log2(", OPER_LOG2, node)) return;
    if (TreatTerm("log10(", OPER_LOG10, node)) return;

    if (TreatTerm("sin(", OPER_SIN, node)) return;
    if (TreatTerm("sqrt(", OPER_SQRT, node)) return;
    if (TreatTerm("sinh(", OPER_SINH, node)) return;

    if (TreatTerm("tan(", OPER_TAN, node)) return;
    if (TreatTerm("tanh(", OPER_TANH, node)) return;
        
	if (std::string term = "AVG("; iBeginsWith(expression, term)) {
		std::string avgExpression;
		avgExpression += currentChar;
		while (currentChar != ')') {
			AV();
			avgExpression += currentChar;
		};
		AV();
        auto varIterator = AddVar(avgExpression);
		AddNode(TVARIABLE, varIterator, node, nullptr, nullptr);
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
		AddNode(TVARIABLE, varIterator, node, nullptr, nullptr);
	} else if (std::string term = "pi"; iBeginsWith(expression, term)) {
        AV(term.length());
        AddNode(TDOUBLE, M_PI, node, nullptr, nullptr);
    }
    else if (std::string term = "pow("; iBeginsWith(expression, term)) {
        AV(term.length());
        std::shared_ptr<EtreeNode> left; ReadExpression(left);
        if (currentChar != ',') SetError(", expected", currentPos);
        AV();
        std::shared_ptr<EtreeNode> right; ReadExpression(right);
        AddNode(OPER_POW, std::monostate{}, node, left, right);
        if (currentChar != ')') SetError(") expected", currentPos);
        AV();
    }
	else {
        if ((currentChar >= 'A' && currentChar <= 'Z') ||
            (currentChar >= 'a' && currentChar <= 'z') ||
            (currentChar == '_'))
        {
            auto varIterator = AddVar(ReadVariable());
            AddNode(TVARIABLE, varIterator, node, nullptr, nullptr);
        }
        else {
            SetError("Syntax error", currentPos);
        }
	}  
}

void GLFormula::ReadPower(std::shared_ptr<EtreeNode> node) //parse "(left)^(right)", write to node
{
	if (error) return;

    std::shared_ptr<EtreeNode> left;
    ReadTerm(left);
	if (currentChar == '^')
	{
		AV();
        std::shared_ptr<EtreeNode> right;
        ReadTerm(right);
        AddNode(OPER_POWER, std::monostate{}, node, left, right);
	}
	else {
		node = left;
	}
}

void GLFormula::ReadFactor(std::shared_ptr<EtreeNode> node) //write to node
{
    if (error) return;

    std::shared_ptr<EtreeNode> left;
    ReadFactor(left);

    while((currentChar=='*' || currentChar=='/') && !error)
    {
        switch (currentChar) {
        case '*': {
            AV();
            std::shared_ptr<EtreeNode> right;
            ReadPower(right);
            AddNode(OPER_MUL, std::monostate{}, left, left, right);
            break;
        }

        case '/': {
            AV();
            std::shared_ptr<EtreeNode> right;
            ReadPower(right);
            AddNode(OPER_DIV, std::monostate{}, left, left, right);
            break;
        }
        }
    }
    node=left;  
}

void GLFormula::ReadExpression(std::shared_ptr<EtreeNode> node) //write to node
{
	if (error) return;

	std::shared_ptr<EtreeNode> left;
	ReadFactor(left);

	while ((currentChar == '+' || currentChar == '-') && !error)
	{
		switch (currentChar) {
		case '+': {
			AV();
			std::shared_ptr<EtreeNode> right;
			ReadFactor(right);
			AddNode(OPER_PLUS, 0.0, left, left, right);
			break;
		}

		case '-': {
			AV();
            std::shared_ptr<EtreeNode> right;
            ReadFactor(right);
			AddNode(OPER_MINUS, 0.0, left, left, right);
			break;
		}
		}
	}
	node = left;
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

	evalTree=std::make_shared<EtreeNode>(0.0);
	varList.clear();

	ReadExpression(evalTree);

    if (currentPos != (int)expression.length()) { //couldn't parse till end
        SetError("Syntax error", currentPos);
    }

	if (error) {
        evalTree=nullptr;
        varList.clear();
	}

	return !error;
}

double factorial(double x) {

  int f = (int)(x+0.5);
  size_t r = 1;
  for(int i=1;i<=f;i++) {
    r = (size_t)i * r;
  }
  return (double)r;

}

double GLFormula::EvalTree(std::shared_ptr<EtreeNode> node) {

  double a,b,r;
  r=0.0;

  if(!error) 
  switch( node->type ) {
   case OPER_PLUS : 
        a=EvalTree(node->left);
        b=EvalTree(node->right);
        r=a+b;
        break;
   case OPER_MINUS: 
        a=EvalTree(node->left);
        b=EvalTree(node->right);
        r=a-b;
        break;
   case OPER_MUL:   
        a=EvalTree(node->left);
        b=EvalTree(node->right);
        r=a*b;
        break;
   case OPER_DIV:   
        a=EvalTree(node->left);
        b=EvalTree(node->right);
        if(b==0.0) {
          error=true;
          errMsg="Divide by 0";
        } else {
          r=a/b;
        }
        break;
   case OPER_POWER: 
       a=EvalTree(node->left);
       b=EvalTree(node->right);
       r=pow(a,b);
       if( errno!=0 ) {
          error=true;
          errMsg = strerror(errno);
       }
       break;
   case OPER_COS:  
       a=EvalTree(node->left);
       r=cos(a);
       if( errno!=0 ) {
          error=true;
          errMsg = strerror(errno);
       }
       break;
   case OPER_CI95:  
        a=EvalTree(node->left);
        b=EvalTree(node->right);
        r=1.96*sqrt(a*(1.0-a)/b);
        break;
   case OPER_POW:  
        a=EvalTree(node->left);
        b=EvalTree(node->right);
        r=pow(a,b);
        break;
   case OPER_FACT:  a=EvalTree(node->left);
       r=factorial(a);
       if( errno!=0 ) {
          error=true;
          errMsg = strerror(errno);
       }
       break;
   case OPER_ACOS:  a=EvalTree(node->left);
       r=acos(a);
       if( errno!=0 ) {
          error=true;
          errMsg = strerror(errno);
       }
       break;
   case OPER_SIN:  a=EvalTree(node->left);
       r=sin(a);
       if( errno!=0 ) {
          error=true;
          errMsg = strerror(errno);
       }
       break;
   case OPER_ASIN:  a=EvalTree(node->left);
       r=asin(a);
       if( errno!=0 ) {
          error=true;
          errMsg = strerror(errno);
       }
       break;
   case OPER_COSH:  a=EvalTree(node->left);
       r=cosh(a);
       if( errno!=0 ) {
          error=true;
          errMsg = strerror(errno);
       }
       break;
   case OPER_SINH:  a=EvalTree(node->left);
       r=sinh(a);
       if( errno!=0 ) {
          error=true;
          errMsg = strerror(errno);
       }
       break;
   case OPER_EXP:   a=EvalTree(node->left);
       r=exp(a);
       if( errno!=0 ) {
          error=true;
          errMsg = strerror(errno);
       }
       break;
   case OPER_LN:   a=EvalTree(node->left);
       r=log(a);
       if( errno!=0 ) {
          error=true;
          errMsg = strerror(errno);
       }
       break;
   case OPER_LOG10:   a=EvalTree(node->left);
       r=log10(a);
       if( errno!=0 ) {
          error=true;
          errMsg = strerror(errno);
       }
       break;
   case OPER_LOG2:   a=EvalTree(node->left);
       r=log(a)/log(2.0);
       if( errno!=0 ) {
          error=true;
          errMsg = strerror(errno);
       }
       break;
   case OPER_SQRT: a=EvalTree(node->left);
       r=sqrt(a);
       if( errno!=0 ) {
          error=true;
          errMsg = strerror(errno);
       }
       break;
   case OPER_TAN:  a=EvalTree(node->left);
       r=tan(a);
       if( errno!=0 ) {
          error=true;
          errMsg = strerror(errno);
       }
       break;
   case OPER_ATAN:  a=EvalTree(node->left);
       r=atan(a);
       if( errno!=0 ) {
          error=true;
          errMsg = strerror(errno);
       }
       break;
   case OPER_TANH:  a=EvalTree(node->left);
       r=tanh(a);
       if( errno!=0 ) {
          error=true;
          errMsg = strerror(errno);
       }
       break;
   case OPER_ABS:  a=EvalTree(node->left);
       r=fabs(a);
       break;
   case OPER_MINUS1: a=EvalTree(node->left);
       r=-a;
       break;
   case TDOUBLE:   r=std::get<double>(node->value);
       break;
   case TVARIABLE: r=std::get<std::list<Variable>::iterator>(node->value)->value;
       break;
  }

  return r;
}

size_t GLFormula::GetNbVariable() {
    return varList.size();
}

std::list<Variable>::iterator GLFormula::GetVariableAt(size_t n) {
    auto it = varList.begin();
    std::advance(it, std::min(n,varList.size()-1));
    return it;
}

void   GLFormula::SetVariable(const std::string& name,double value) {
  auto v = FindVar(name);
  if( v!=varList.end() ) v->value = value;
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

bool GLFormula::Evaluate(double *result)
{
  error=false;
  errno=0;

  /* Evaluate expression */
  
  if(evalTree)
  {
    *result=EvalTree(evalTree);
  } else {
    error=true;
  }

  return !error;
}