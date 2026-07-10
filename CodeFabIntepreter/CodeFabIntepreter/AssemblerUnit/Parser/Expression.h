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

class CallExpr : public Expression {
public:
	CallExpr(unique_ptr<Expression> callee, const Token& paren, vector<unique_ptr<Expression>> arguments)
		: callee{ move(callee) }, paren{ paren }, arguments{ move(arguments) } {}

	const Expression* getCallee() const {
		return callee.get();
	}

	const Token& getParen() const {
		return paren;
	}

	const vector<unique_ptr<Expression>>& getArguments() const {
		return arguments;
	}

	void accept(ExprVisitor& visitor) const override;

private:
	unique_ptr<Expression> callee;
	Token paren;
	vector<unique_ptr<Expression>> arguments;
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

class GetExpr : public Expression {
public:
	GetExpr(unique_ptr<Expression> object, const Token& name) : object{ move(object) }, name{ name } {}

	const Expression* getObject() const {
		return object.get();
	}

	const Token& getName() const {
		return name;
	}

	// Parser가 `a.b = c`를 파싱할 때, 이미 만들어진 GetExpr(a.b)을 SetExpr로 바꾸기 위해
	// object 소유권을 꺼내 쓴다. 트리 조립 중에만 쓰이며 완성된 트리를 순회하는
	// Checker/Interpreter는 이 메서드를 호출하지 않는다.
	unique_ptr<Expression> releaseObject() {
		return move(object);
	}

	void accept(ExprVisitor& visitor) const override;

private:
	unique_ptr<Expression> object;
	Token name;
};

class SetExpr : public Expression {
public:
	SetExpr(unique_ptr<Expression> object, const Token& name, unique_ptr<Expression> value)
		: object{ move(object) }, name{ name }, value{ move(value) } {}

	const Expression* getObject() const {
		return object.get();
	}

	const Token& getName() const {
		return name;
	}

	const Expression* getValue() const {
		return value.get();
	}

	void accept(ExprVisitor& visitor) const override;

private:
	unique_ptr<Expression> object;
	Token name;
	unique_ptr<Expression> value;
};

class ThisExpr : public Expression {
public:
	explicit ThisExpr(const Token& keyword) : keyword{ keyword } {}

	const Token& getKeyword() const {
		return keyword;
	}

	void accept(ExprVisitor& visitor) const override;

private:
	Token keyword;
};

class SuperExpr : public Expression {
public:
	SuperExpr(const Token& keyword, const Token& method) : keyword{ keyword }, method{ method } {}

	const Token& getKeyword() const {
		return keyword;
	}

	const Token& getMethod() const {
		return method;
	}

	void accept(ExprVisitor& visitor) const override;

private:
	Token keyword;
	Token method;
};

class InstanceOfExpr : public Expression {
public:
	InstanceOfExpr(unique_ptr<Expression> object, const Token& keyword, const Token& class_name)
		: object{ move(object) }, keyword{ keyword }, class_name{ class_name } {}

	const Expression* getObject() const {
		return object.get();
	}

	const Token& getKeyword() const {
		return keyword;
	}

	const Token& getClassName() const {
		return class_name;
	}

	void accept(ExprVisitor& visitor) const override;

private:
	unique_ptr<Expression> object;
	Token keyword;
	Token class_name;
};
