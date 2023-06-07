// Copyright (c) 2011 rubicon IT GmbH

/* Supported grammar :                                           */
/*                                                              */
/* <expr>   ::= <factor> [ <oper1> <factor> ]*                  */
/* <factor> ::= <power>  [ <oper2> <power> ]*                   */
/* <power>  ::= <term> [ <oper3> <term> ]                       */
/* <term>   ::= '(' <exp> ')'      |                            */
/*                <double>         |                            */
/*                <variable>       |                            */
/*               '-' <term>        |                            */
/*               'sin(' <exp> ')'  |                            */
/*               'cos(' <exp> ')'  |                            */
/*               'tan(' <exp> ')'  |                            */
/*               'asin(' <exp> ')' |                            */
/*               'acos(' <exp> ')' |                            */
/*               'atan(' <exp> ')' |                            */
/*               'sinh(' <exp> ')' |                            */
/*               'cosh(' <exp> ')' |                            */
/*               'tanh(' <exp> ')' |                            */
/*               'exp(' <exp> ')'  |                            */
/*               'ln(' <exp> ')'   |                            */
/*               'log(' <exp> ')'  |                            */
/*               'inv(' <exp> ')'  |                            */
/*               'sqrt(' <exp> ')' |                            */
/*               'abs(' <exp> ')'  |                            */
/*               'fact(' <exp> ')'  |                           */
/*               'pow(' <exp>,<exp> ')'  |                      */
/*               'ci95(' <exp>,<exp> ')' |                      */
/*               'sum(' <variable>,<double>,<double> ')' |      */
/*               'PI'                                           */
/* <oper1>    ::= '+' | '-'                                     */
/* <oper2>    ::= '*' | '/'                                     */
/* <oper3>    ::= '^'                                           */
/* <double>   ::= <nb>* '.' <nb> <nb>* ['E' [-] <nb> <nb>*]     */
/* <nb>       ::= '0'..'9'                                      */
/* <variable> ::= <letter>[<letter> | <nb>]*                    */
/* <letter>   ::= 'A'..'Z' | 'a'..'z' | '_'                     */

#ifndef _GLPARSERH_
#define _GLPARSERH_

#include "GLTypes.h" // For bool typedef
#include <string>
#include <variant>
#include <list>
#include <memory>

// Evaluation tree node type
#define OPER_PLUS   1
#define OPER_MINUS  2
#define OPER_MUL    3
#define OPER_DIV    4
#define OPER_POWER   5
#define OPER_COS    6
#define OPER_SIN    7
#define OPER_TAN    8
#define OPER_ACOS   9
#define OPER_ASIN   10
#define OPER_ATAN   11
#define OPER_COSH   12
#define OPER_SINH   13
#define OPER_TANH   14
#define OPER_EXP    15
#define OPER_LN     16
#define OPER_LOG2   17
#define OPER_LOG10  18
#define OPER_INV    19
#define OPER_SQRT   20
#define OPER_ABS    21
#define OPER_MINUS1 22
#define OPER_FACT   23
#define OPER_CI95   24
#define OPER_POW    25

#define TDOUBLE     50
#define TVARIABLE   51

struct Variable {
	std::string varName;
	double value;
};

//Evaluation tree node
struct EtreeNode {
	int type; //operand, value, etc.
	std::variant<std::monostate, double, std::list<Variable>::iterator> value; //constant value, evaluated variable, or nothing (monostate) in case of operand
	std::shared_ptr<EtreeNode> left = nullptr;
	std::shared_ptr<EtreeNode> right = nullptr;
	template <typename T>
	EtreeNode(T val) : value(val) {} //accepts either double or Variable as value
};

class GLFormula {

public:

	//Optional formula name
	void SetName(const std::string& name);
	std::string GetName();

	// Expression management
	void SetExpression(const std::string& expression);  // Set formula expression
	std::string GetExpression();     // Get the expression
	bool Parse();                    // Construct eval tree
	std::string GetErrorMsg();       // Return error message
	int   GetCurrentPos();           // Current parsing cursor position

	// Variables
	size_t    GetNbVariable();          // Return num of variables created during parsing
	std::list<Variable>::iterator GetVariableAt(size_t n);   // Return iterator to nth variable
	void   SetVariable(const std::string&, double value); // Set the variable value

	std::string GetVariableEvalError();
	void SetVariableEvalError(const std::string& errMsg);

	// Evaluation
	bool   Evaluate(double* result); // Evaluate the expression (after it was parsed)
	bool   hasVariableEvalError;
	std::string variableEvalErrorMsg;

private:

	double EvalTree(std::shared_ptr<EtreeNode>& node);
	void   ReadExpression(std::shared_ptr<EtreeNode>& node);
	bool   TreatTerm(const std::string& term, int operand, std::shared_ptr<EtreeNode>& node);
	void   ReadTerm(std::shared_ptr<EtreeNode>& node);
	void   ReadPower(std::shared_ptr<EtreeNode>& node);
	void   ReadFactor(std::shared_ptr<EtreeNode>& node);
	std::string ReadVariable();
	void   ReadDouble(double* R);
	void   AddNode(int type, std::variant<std::monostate, double, std::list<Variable>::iterator> value, std::shared_ptr<EtreeNode>& node, std::shared_ptr<EtreeNode> left, std::shared_ptr<EtreeNode> right);
	std::list<Variable>::iterator AddVar(const std::string& var_name);
	std::list<Variable>::iterator GLFormula::FindVar(const std::string& var_name);
	void   SetError(const std::string& errMsg, int pos);
	void   AV(size_t times = 1); //advance by one (or more) non-whitespace char
	std::string Extract(int n); //Left n characters of string

	std::string name;   // Name (optional)
	std::string errMsg; // Error message
	std::string expression;  // Expression
	char currentChar;          // Current parsed char
	int  currentPos;     // Current char pos
	bool error = false;       // Error flag

	std::shared_ptr<EtreeNode> evalTree = nullptr;
	std::list<Variable> varList;

};

#endif /* _GLPARSERH_ */