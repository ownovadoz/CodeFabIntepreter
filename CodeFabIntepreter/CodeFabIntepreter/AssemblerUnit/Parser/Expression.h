#pragma once

#include "../Tokenizer/Token.h"
#include "../../Node.h"

#include <memory>
#include <vector>

using std::shared_ptr;
using std::vector;

class ExpressionOrToken : public Node {
};

class Expression : public ExpressionOrToken {
private:
	vector<ExpressionOrToken*> children;
};

class LiteralExpr : public Expression {
public:
	explicit LiteralExpr(const Token& token) : value(token) {}

	void accept(Visitor& v) override;

	Token getValue() const { return value; }

private:
	Token value;
};

class VariableExpr : public Expression {
};

class AssignExpr : public Expression {
};

class BinaryExpr : public Expression {
public:
	BinaryExpr(const shared_ptr<Expression>& left, const Token& op, const shared_ptr<Expression>& right)
		: left(left), op(op), right(right) {}

	void accept(Visitor& v) override;

	shared_ptr<Expression> getLeft() const { return left; }
	Token getOp() const { return op; }
	shared_ptr<Expression> getRight() const { return right; }

private:
	shared_ptr<Expression> left;
	Token op;
	shared_ptr<Expression> right;
};

class UnaryExpr : public Expression {
};

class GroupingExpr : public Expression {
};

class LogicalExpr : public Expression {
};