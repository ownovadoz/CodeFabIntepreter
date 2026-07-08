#pragma once

#include "Node.h"
#include "Expression.h"
#include "../Tokenizer/Token.h"

#include <memory>
#include <vector>

using std::move;
using std::unique_ptr;
using std::vector;

class Statement : public StatementOrExpression {
public:
	virtual void accept(StmtVisitor& visitor) {}
};

class ExpressionStmt : public Statement {
public:
	void accept(StmtVisitor& visitor) override;
private:
	unique_ptr<Expression> expr;
};

class IfStmt : public Statement {
public:
	IfStmt(unique_ptr<BinaryExpr> condition, unique_ptr<Statement> then_branch, unique_ptr<Statement> else_branch)
		: condition{ move(condition) }, then_branch{ move(then_branch) }, else_branch{ move(else_branch) } {}

	const BinaryExpr* getCondition() const {
		return condition.get();
	}

	Statement* getThenBranch() const {
		return then_branch.get();
	}

	Statement* getElseBranch() const {
		return else_branch.get();
	}

	void accept(StmtVisitor& visitor) override;

private:
	unique_ptr<BinaryExpr> condition;	// Enforce binary expressions only.
	unique_ptr<Statement> then_branch;
	unique_ptr<Statement> else_branch;
};

class BlockStmt : public Statement {
public:
	void addStatement(unique_ptr<Statement> stmt) {
		statements.push_back(move(stmt));
	}

	const vector<unique_ptr<Statement>>& getStatements() const {
		return statements;
	}

	void accept(StmtVisitor& visitor) override;

private:
	vector<unique_ptr<Statement>> statements;
};

class VarDeclareStmt : public Statement {
public:
	explicit VarDeclareStmt(const Token& token) : name{ token } {}

	void setExpression(unique_ptr<Expression> expr) {
		initializer = move(expr);
	}

	const Token& getName() {
		return name;
	}

	const Expression* getInitializer() {
		return initializer.get();
	}

	void accept(StmtVisitor& visitor) override;

private:
	Token name;
	unique_ptr<Expression> initializer;
};

class PrintStmt : public Statement {
public:
	void accept(StmtVisitor& visitor) override;
private:
	unique_ptr<Expression> expr;
};

class ForStmt : public Statement {
public:
	void accept(StmtVisitor& visitor) override;
private:
	unique_ptr<VarDeclareStmt> init;
	unique_ptr<BinaryExpr> condition;
	unique_ptr<Expression> increment;
	unique_ptr<Statement> body;
};
