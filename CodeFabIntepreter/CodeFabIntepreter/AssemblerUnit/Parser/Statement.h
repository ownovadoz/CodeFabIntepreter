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
	virtual void accept(StmtVisitor& visitor) const = 0;
};

class ExpressionStmt : public Statement {
public:
	explicit ExpressionStmt(unique_ptr<Expression> expr) : expr{ move(expr) } {}

	const Expression* getExpr() const {
		return expr.get();
	}

	void accept(StmtVisitor& visitor) const override;

private:
	unique_ptr<Expression> expr;
};

class IfStmt : public Statement {
public:
	IfStmt(unique_ptr<Expression> condition, unique_ptr<Statement> then_branch, unique_ptr<Statement> else_branch)
		: condition{ move(condition) }, then_branch{ move(then_branch) }, else_branch{ move(else_branch) } {}

	const Expression* getCondition() const {
		return condition.get();
	}

	const Statement* getThenBranch() const {
		return then_branch.get();
	}

	const Statement* getElseBranch() const {
		return else_branch.get();
	}

	void accept(StmtVisitor& visitor) const override;

private:
	unique_ptr<Expression> condition;
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

	void accept(StmtVisitor& visitor) const override;

private:
	vector<unique_ptr<Statement>> statements;
};

class VarDeclareStmt : public Statement {
public:
	explicit VarDeclareStmt(const Token& token) : name{ token } {}

	void setExpression(unique_ptr<Expression> expr) {
		initializer = move(expr);
	}

	const Token& getName() const {
		return name;
	}

	const Expression* getInitializer() const {
		return initializer.get();
	}

	void accept(StmtVisitor& visitor) const override;

private:
	Token name;
	unique_ptr<Expression> initializer;
};

class PrintStmt : public Statement {
public:
	explicit PrintStmt(unique_ptr<Expression> expr) : expr{ move(expr) } {}

	const Expression* getExpr() const {
		return expr.get();
	}

	void accept(StmtVisitor& visitor) const override;

private:
	unique_ptr<Expression> expr;
};

class ForStmt : public Statement {
public:
	ForStmt(unique_ptr<VarDeclareStmt> init, unique_ptr<Expression> condition, unique_ptr<Expression> increment, unique_ptr<Statement> body)
		: init{ move(init) }, condition{ move(condition) }, increment{ move(increment) }, body{ move(body) } {}

	const VarDeclareStmt* getInit() const {
		return init.get();
	}

	const Expression* getCondition() const {
		return condition.get();
	}

	const Expression* getIncrement() const {
		return increment.get();
	}

	const Statement* getBody() const {
		return body.get();
	}

	void accept(StmtVisitor& visitor) const override;

private:
	unique_ptr<VarDeclareStmt> init;
	unique_ptr<Expression> condition;
	unique_ptr<Expression> increment;
	unique_ptr<Statement> body;
};
