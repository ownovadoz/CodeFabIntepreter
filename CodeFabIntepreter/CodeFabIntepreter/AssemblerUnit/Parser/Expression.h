#pragma once

#include "../../Node.h"

class ExpressionOrToken : public Node {
};

class Expression : public ExpressionOrToken {
private:
	vector<ExpressionOrToken*> children;
};

class LiteralExpr : public Expression {
};

class VariableExpr : public Expression {
};

class AssignExpr : public Expression {
};

class BinaryExpr : public Expression {
};

class UnaryExpr : public Expression {
};

class GroupingExpr : public Expression {
};

class LogicalExpr : public Expression {
};