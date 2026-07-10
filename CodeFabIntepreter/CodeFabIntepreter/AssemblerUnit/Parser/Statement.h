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

class FunctionStmt : public Statement {
public:
	FunctionStmt(const Token& name, vector<Token> params, unique_ptr<BlockStmt> body)
		: name{ name }, params{ move(params) }, body{ move(body) } {}

	const Token& getName() const {
		return name;
	}

	const vector<Token>& getParams() const {
		return params;
	}

	const BlockStmt* getBody() const {
		return body.get();
	}

	// init은 클래스 생성자를 나타내는 예약된 메서드 이름이다(별도 키워드 없이 이름으로 식별).
	bool isInitializer() const {
		return name.getLexeme() == "init";
	}

	void accept(StmtVisitor& visitor) const override;

private:
	Token name;
	vector<Token> params;
	unique_ptr<BlockStmt> body;
};

class ReturnStmt : public Statement {
public:
	ReturnStmt(const Token& keyword, unique_ptr<Expression> value) : keyword{ keyword }, value{ move(value) } {}

	const Token& getKeyword() const {
		return keyword;
	}

	const Expression* getValue() const {
		return value.get();
	}

	void accept(StmtVisitor& visitor) const override;

private:
	Token keyword;
	unique_ptr<Expression> value;
};

class ClassStmt : public Statement {
public:
	ClassStmt(const Token& name, unique_ptr<VariableExpr> superclass, vector<unique_ptr<FunctionStmt>> methods)
		: name{ name }, superclass{ move(superclass) }, methods{ move(methods) } {}

	const Token& getName() const {
		return name;
	}

	const VariableExpr* getSuperclass() const {
		return superclass.get();
	}

	const vector<unique_ptr<FunctionStmt>>& getMethods() const {
		return methods;
	}

	void accept(StmtVisitor& visitor) const override;

private:
	Token name;
	unique_ptr<VariableExpr> superclass;
	vector<unique_ptr<FunctionStmt>> methods;
};

class ImportStmt : public Statement {
public:
	ImportStmt(const Token& path, const Token& alias) : path{ path }, alias{ alias } {}

	const Token& getPath() const {
		return path;
	}

	const Token& getAlias() const {
		return alias;
	}

	void accept(StmtVisitor& visitor) const override;

private:
	Token path;
	Token alias;
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
