#pragma once

#include <vector>

using std::vector;

class StatementOrExpression {
public:
	virtual ~StatementOrExpression() = default;
};

class Expression : public StatementOrExpression {
public:
	virtual ~Expression() = default;
};