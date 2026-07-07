#pragma once

#include "../../Node.h"
#include "../Tokenize/Token.h"
#include "Expression.h"

#include <vector>

using std::vector;

class Statement : public Node {
private:
	vector<Node*> children;
};

class ExpressionStmt : public Statement {
};

class PrintStmt : public Statement {
};

class VarDeclareStmt : public Statement {
	Token* name;
	LiteralExpr* initializer;
};

class BlockStmt : public Statement {
private:
	vector<ExpressionStmt*> statements;
};

class IfStmt : public Statement {
	Expression* condition;
	ExpressionStmt* then_branch;
	ExpressionStmt* else_branch;
};

class ForStmt : public Statement {
};