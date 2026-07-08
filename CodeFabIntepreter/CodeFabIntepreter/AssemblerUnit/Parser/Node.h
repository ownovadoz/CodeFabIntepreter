#pragma once

#include <vector>

using std::vector;

class ExprVisitor;
class StmtVisitor;

class StatementOrExpression {
public:
	virtual ~StatementOrExpression() = default;
};

class Expression : public StatementOrExpression {
public:
	virtual ~Expression() = default;
	virtual void accept(ExprVisitor& visitor) const {}
};
