#pragma once

#include "Statement.h"
#include "../Tokenizer/Token.h"

#include <vector>

using std::vector;

class Parser
{
public:
	Statement* parse(const vector<Token>& tokens);


private:
	Statement* parseStatement();
	Statement* parseIfStmt();
	Statement* parseBlockStmt();
	Statement* parseVarDeclareStmt();
	Statement* parsePrintStmt();
	Statement* parseForStmt();
	Statement* parseExpressionStmt();

	Expression* parseExpression();
	Expression* parseNumberExpr();

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

	vector<Token> tokens;
	vector<Token>::const_iterator current_token_it;
};