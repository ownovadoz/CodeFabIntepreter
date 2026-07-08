#pragma once

#include "Node.h"
#include "../Tokenizer/Token.h"

class LiteralExpr : public Expression {
public:
	explicit LiteralExpr(const Token& token) : token{ token } {}
	const Token& getToken() const {
		return token;
	}
private:
	Token token;
};

class VariableExpr : public Expression {
private:
	Token token;
};

class AssignExpr : public Expression {
private:
	Token identifier;
	Expression* assign_expr;
	Expression* expr;
};

class BinaryExpr : public Expression {
private:
	Expression* op;
	Expression* left;
	Expression* right;
};

class UnaryExpr : public Expression {
public:
	explicit UnaryExpr(const Token& op, Expression* expr) : op{ op }, expr{ expr } {}

	~UnaryExpr() override {
		delete expr;
	}

	const Token& getOperator() const {
		return op;
	}

	const Expression* getExpr() const {
		return expr;
	}

private:
	Token op;
	Expression* expr;
};

class GroupingExpr : public Expression {
private:
	Expression* expr;	// TODO: is it right?
};

class LogicalExpr : public Expression {
private:
	Expression* expr;
	Expression* left;
	Expression* right;
};