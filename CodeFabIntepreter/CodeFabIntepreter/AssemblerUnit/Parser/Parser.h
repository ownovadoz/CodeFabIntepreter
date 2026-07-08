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
	unique_ptr<Statement> parseExpressionStmt();

	unique_ptr<Expression> parseExpression();
	unique_ptr<Expression> parseAssignExpr();
	unique_ptr<Expression> parseLogicOr();
	unique_ptr<Expression> parseLogicAnd();
	unique_ptr<Expression> parseEquality();
	unique_ptr<Expression> parseComparison();
	unique_ptr<Expression> parseTerm();
	unique_ptr<Expression> parseFactor();
	unique_ptr<Expression> parseUnaryExpr();
	unique_ptr<Expression> parsePrimaryExpr();

	unique_ptr<Expression> parseBinaryExpr(unique_ptr<Expression> (Parser::* parseOperand)(), initializer_list<TokenType> operators);

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
