#pragma once

#include <string>
#include <variant>
#include <list>
#include <memory>
#include <optional>
#include <map>

// Evaluation tree node type
enum class OperandType : int {
	PLUS,
	MINUS,
	MUL,
	DIV,
	POWER,
	COS,
	SIN,
	TAN,
	ACOS,
	ASIN,
	ATAN,
	COSH,
	SINH,
	TANH,
	EXP,
	LN,
	LOG2,
	LOG10,
	INV,
	SQRT,
	ABS,
	MINUS1,
	FACT,
	CI95,
	POW,

	TDOUBLE,
	TVARIABLE
};

struct Variable {
	std::string varName;
	double value;
};

//Evaluation tree node
struct EvalTreeNode {
	OperandType type;
	std::variant<std::monostate, double, std::list<Variable>::iterator> value; //constant value, evaluated variable, or nothing (monostate) in case of operand
	std::unique_ptr<EvalTreeNode> left = nullptr;
	std::unique_ptr<EvalTreeNode> right = nullptr;
	template <typename T> EvalTreeNode(T val) : value(val) {} //accepts either double or Variable or "nothing" (monostate) as value

	// Copy constructor
	EvalTreeNode(const EvalTreeNode& other)
		: type(other.type), value(other.value)
	{
		if (other.left) {
			left = std::make_unique<EvalTreeNode>(*other.left);
		}
		if (other.right) {
			right = std::make_unique<EvalTreeNode>(*other.right);
		}
	}

	// Copy assignment operator
	EvalTreeNode& operator=(const EvalTreeNode& other)
	{
		if (this != &other) {
			type = other.type;
			value = other.value;

			left.reset();
			right.reset();

			if (other.left) {
				left = std::make_unique<EvalTreeNode>(*other.left);
			}
			if (other.right) {
				right = std::make_unique<EvalTreeNode>(*other.right);
			}
		}

		return *this;
	}
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
	std::optional<double> EvaluateNode(const std::unique_ptr<EvalTreeNode>& node); // Evaluate the expression (after it was parsed)
	std::optional<double> Evaluate();
	bool   hasVariableEvalError;
	std::string variableEvalErrorMsg;

	static const std::map<std::string, OperandType> mathExpressionsMap;

private:

	double EvalTree(std::shared_ptr<EvalTreeNode>& node);
	std::optional< std::unique_ptr<EvalTreeNode> > TreatTerm(const std::string& term, OperandType operand);
	std::unique_ptr<EvalTreeNode> ReadPlusMinus();
	std::unique_ptr<EvalTreeNode> ReadTerm();
	std::unique_ptr<EvalTreeNode> ReadPower();
	std::unique_ptr<EvalTreeNode> ReadFactor();
	std::string ReadVariable();
	std::optional<double> ReadDouble();
	std::unique_ptr<EvalTreeNode>   AddNode(OperandType type, std::variant<std::monostate, double, std::list<Variable>::iterator> value, std::unique_ptr<EvalTreeNode> left, std::unique_ptr<EvalTreeNode> right);
	std::list<Variable>::iterator AddVar(const std::string& var_name);
	std::list<Variable>::iterator FindVar(const std::string& var_name);
	void   SetError(const std::string& errMsg, int pos);
	void   AV(size_t times = 1); //advance by one (or more) non-whitespace char
	std::string Extract(int n); //Left n characters of string

	std::string name;   // Name (optional)
	std::string errMsg; // Error message
	std::string expression;  // Expression
	char currentChar;          // Current parsed char
	int  currentPos;     // Current char pos
	bool error = false;       // Error flag

	std::unique_ptr<EvalTreeNode> evalTree;
	std::list<Variable> varList;

};