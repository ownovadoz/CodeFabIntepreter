#pragma once

#include "Node.h"
#include "Expression.h"
#include "../Tokenizer/Token.h"

#include <vector>

using std::vector;

class Statement : public StatementOrExpression {
};

class ExpressionStmt : public Statement {
private:
	Expression* expr = nullptr;
};

class IfStmt : public Statement {
private:
	BinaryExpr* condition = nullptr;	// Enforce binary expressions only.
	Statement* then_branch = nullptr;
	Statement* else_branch = nullptr;
};

class BlockStmt : public Statement {
private:
	vector<Statement*> exprs;
};

class VarDeclareStmt : public Statement {
public:
	explicit VarDeclareStmt(const Token& token) : name{ token }, initializer{ nullptr } {}
	
	void setExpression(Expression* expr) {
		initializer = expr;
	}

	const Token& getName() {
		return name;
	}

	const Expression* getInitializer() {
		return initializer;
	}

private:
	Token name;
	Expression* initializer = nullptr;
};

class PrintStmt : public Statement {
private:
	Expression* expr = nullptr;
};

class ForStmt : public Statement {
private:
	VarDeclareStmt* init = nullptr;
	BinaryExpr* condition = nullptr;
	Expression* increment = nullptr;
	Statement* body = nullptr;
};