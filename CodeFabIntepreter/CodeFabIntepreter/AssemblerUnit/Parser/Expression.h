#pragma once

#include "Node.h"
#include "../Tokenizer/Token.h"

#include <memory>
#include <utility>

using std::move;
using std::unique_ptr;

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
public:
	explicit VariableExpr(const Token& token) : token{ token } {}

	const Token& getToken() const {
		return token;
	}

private:
	Token token;
};

class AssignExpr : public Expression {
private:
	Token identifier;
	unique_ptr<Expression> assign_expr;
	unique_ptr<Expression> expr;
};

class BinaryExpr : public Expression {
private:
	unique_ptr<Expression> op;
	unique_ptr<Expression> left;
	unique_ptr<Expression> right;
};

class UnaryExpr : public Expression {
public:
	UnaryExpr(const Token& op, unique_ptr<Expression> expr) : op{ op }, expr{ move(expr) } {}

	const Token& getOperator() const {
		return op;
	}

	const Expression* getExpr() const {
		return expr.get();
	}

private:
	Token op;
	unique_ptr<Expression> expr;
};

class GroupingExpr : public Expression {
public:
	explicit GroupingExpr(unique_ptr<Expression> expr) : expr{ move(expr) } {}

	const Expression* getExpr() const {
		return expr.get();
	}

private:
	unique_ptr<Expression> expr;
};

class LogicalExpr : public Expression {
private:
	unique_ptr<Expression> expr;
	unique_ptr<Expression> left;
	unique_ptr<Expression> right;
};
