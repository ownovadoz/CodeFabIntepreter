#pragma once

#include "../../Node.h"
#include "../Tokenizer/Token.h"
#include "Expression.h"

#include <vector>
#include <memory>

using std::vector;
using std::shared_ptr;
using std::make_shared;

class Statement : public Node {
private:
	vector<Node*> children;
};

class ExpressionStmt : public Statement {
};

class PrintStmt : public Statement {
};

class VarDeclareStmt : public Statement {
public:
	void accept(Visitor& v) override;

	void setName(const Token& token) {
		name = make_shared<Token>(token);
	}

	void setInitializer(const shared_ptr<Expression>& expr) {
		initializer = expr;
	}

	shared_ptr<Token> getName() const { return name; }
	shared_ptr<Expression> getInitializer() const { return initializer; }

private:
	shared_ptr<Token> name;
	shared_ptr<Expression> initializer;
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