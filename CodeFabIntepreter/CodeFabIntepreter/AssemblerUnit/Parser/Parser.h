#pragma once

#include "Statement.h"
#include "../Tokenizer/Token.h"

#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

using std::initializer_list;
using std::string;
using std::unique_ptr;
using std::vector;

class Parser
{
public:
	vector<unique_ptr<Statement>> parse(const vector<Token>& tokens);


private:
	unique_ptr<Statement> parseStatement();
	unique_ptr<Statement> parseIfStmt();
	unique_ptr<Statement> parseBlockStmt();
	unique_ptr<Statement> parseVarDeclareStmt();
	unique_ptr<Statement> parsePrintStmt();
	unique_ptr<Statement> parseForStmt();
	unique_ptr<Statement> parseFunctionStmt();
	unique_ptr<FunctionStmt> finishFunctionDecl(const Token& name);
	unique_ptr<Statement> parseReturnStmt();
	unique_ptr<Statement> parseClassStmt();
	unique_ptr<Statement> parseExpressionStmt();

	unique_ptr<BlockStmt> parseBlock();
	vector<Token> parseParameters();

	unique_ptr<Expression> parseExpression();
	unique_ptr<Expression> parseAssignExpr();
	unique_ptr<Expression> parseLogicOr();
	unique_ptr<Expression> parseLogicAnd();
	unique_ptr<Expression> parseEquality();
	unique_ptr<Expression> parseInstanceOf();
	unique_ptr<Expression> parseComparison();
	unique_ptr<Expression> parseTerm();
	unique_ptr<Expression> parseFactor();
	unique_ptr<Expression> parseUnaryExpr();
	unique_ptr<Expression> parseCallExpr();
	unique_ptr<Expression> finishCallExpr(unique_ptr<Expression> callee);
	unique_ptr<Expression> finishIndexExpr(unique_ptr<Expression> array);
	unique_ptr<Expression> parsePrimaryExpr();
	unique_ptr<Expression> parseArrayExpr();
	void rejectNonNumericLiteral(const Expression* expr, const string& message);

	using ExprFactory = unique_ptr<Expression> (*)(unique_ptr<Expression>, const Token&, unique_ptr<Expression>);

	unique_ptr<Expression> parseLeftAssocExpr(unique_ptr<Expression> (Parser::* parseOperand)(), initializer_list<TokenType> operators, ExprFactory makeExpr);

	void init(const vector<Token>& tokens) {
		this->tokens = tokens;
		current_token_it = this->tokens.begin();
	}

	const Token& peek() const {
		return *current_token_it;
	}

	const Token& advance() {
		if (!isAtEnd()) {
			return *(current_token_it++);
		}
		return *current_token_it;
	}

	bool isAtEnd() const {
		return current_token_it == tokens.end() || current_token_it->getType() == TokenType::END_OF_FILE;
	}

	bool check(TokenType type) const {
		if (isAtEnd()) return false;
		return peek().getType() == type;
	}

	bool checkAny(initializer_list<TokenType> types) const {
		for (TokenType type : types) {
			if (check(type)) return true;
		}
		return false;
	}

	Token consume(TokenType type, const string& message);

	vector<Token> tokens;
	vector<Token>::const_iterator current_token_it;
};
