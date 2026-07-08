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
	void accept(ExprVisitor& visitor) const override;
private:
	Token token;
};

class VariableExpr : public Expression {
public:
	void accept(ExprVisitor& visitor) const override;
private:
	Token token;
};

class AssignExpr : public Expression {
public:
	void accept(ExprVisitor& visitor) const override;
private:
	Token identifier;
	unique_ptr<Expression> assign_expr;
	unique_ptr<Expression> expr;
};

class BinaryExpr : public Expression {
public:
	void accept(ExprVisitor& visitor) const override;
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

	void accept(ExprVisitor& visitor) const override;

private:
	Token op;
	unique_ptr<Expression> expr;
};

class GroupingExpr : public Expression {
public:
	void accept(ExprVisitor& visitor) const override;
private:
	unique_ptr<Expression> expr;	// TODO: is it right?
};

class LogicalExpr : public Expression {
public:
	void accept(ExprVisitor& visitor) const override;
private:
	unique_ptr<Expression> expr;
	unique_ptr<Expression> left;
	unique_ptr<Expression> right;
};
