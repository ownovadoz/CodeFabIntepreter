#pragma once

#include "Node.h"
#include "../Tokenizer/Token.h"

#include <memory>
#include <utility>
#include <vector>

using std::move;
using std::unique_ptr;
using std::vector;

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
	explicit VariableExpr(const Token& token) : token{ token } {}

	const Token& getToken() const {
		return token;
	}

	void accept(ExprVisitor& visitor) const override;

private:
	Token token;
};

class AssignExpr : public Expression {
public:
	AssignExpr(const Token& identifier, unique_ptr<Expression> value) : identifier{ identifier }, value{ move(value) } {}

	const Token& getIdentifier() const {
		return identifier;
	}

	const Expression* getValue() const {
		return value.get();
	}

	void accept(ExprVisitor& visitor) const override;

private:
	Token identifier;
	unique_ptr<Expression> value;
};

class BinaryExpr : public Expression {
public:
	BinaryExpr(unique_ptr<Expression> left, const Token& op, unique_ptr<Expression> right)
		: left{ move(left) }, op{ op }, right{ move(right) } {}

	const Expression* getLeft() const {
		return left.get();
	}

	const Token& getOperator() const {
		return op;
	}

	const Expression* getRight() const {
		return right.get();
	}

	void accept(ExprVisitor& visitor) const override;

private:
	unique_ptr<Expression> left;
	Token op;
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
	explicit GroupingExpr(unique_ptr<Expression> expr) : expr{ move(expr) } {}

	const Expression* getExpr() const {
		return expr.get();
	}

	void accept(ExprVisitor& visitor) const override;

private:
	unique_ptr<Expression> expr;
};

class LogicalExpr : public Expression {
public:
	LogicalExpr(unique_ptr<Expression> left, const Token& op, unique_ptr<Expression> right)
		: left{ move(left) }, op{ op }, right{ move(right) } {}

	const Expression* getLeft() const {
		return left.get();
	}

	const Token& getOperator() const {
		return op;
	}

	const Expression* getRight() const {
		return right.get();
	}

	void accept(ExprVisitor& visitor) const override;

private:
	unique_ptr<Expression> left;
	Token op;
	unique_ptr<Expression> right;
};

class ArrayExpr : public Expression {
public:
	explicit ArrayExpr(unique_ptr<Expression> size) : size{ move(size) } {}

	const Expression* getSize() const {
		return size.get();
	}

	void accept(ExprVisitor& visitor) const override;

private:
	unique_ptr<Expression> size;
};

class IndexExpr : public Expression {
public:
	IndexExpr(unique_ptr<Expression> array, unique_ptr<Expression> index)
		: array{ move(array) }, index{ move(index) } {}

	const Expression* getArray() const {
		return array.get();
	}

	const Expression* getIndex() const {
		return index.get();
	}

	unique_ptr<Expression> releaseArray() {
		return move(array);
	}

	unique_ptr<Expression> releaseIndex() {
		return move(index);
	}

	void accept(ExprVisitor& visitor) const override;

private:
	unique_ptr<Expression> array;
	unique_ptr<Expression> index;
};

class IndexSetExpr : public Expression {
public:
	IndexSetExpr(unique_ptr<Expression> array, unique_ptr<Expression> index, unique_ptr<Expression> value)
		: array{ move(array) }, index{ move(index) }, value{ move(value) } {}

	const Expression* getArray() const {
		return array.get();
	}

	const Expression* getIndex() const {
		return index.get();
	}

	const Expression* getValue() const {
		return value.get();
	}

	void accept(ExprVisitor& visitor) const override;

private:
	unique_ptr<Expression> array;
	unique_ptr<Expression> index;
	unique_ptr<Expression> value;
};

class CallExpr : public Expression {
public:
	CallExpr(unique_ptr<Expression> callee, vector<unique_ptr<Expression>> arguments)
		: callee{ move(callee) }, arguments{ move(arguments) } {}

	const Expression* getCallee() const {
		return callee.get();
	}

	const vector<unique_ptr<Expression>>& getArguments() const {
		return arguments;
	}

	void accept(ExprVisitor& visitor) const override;

private:
	unique_ptr<Expression> callee;
	vector<unique_ptr<Expression>> arguments;
};
