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
		it = this->tokens.begin();
	}

	const Token& peek() const {
		return *it;
	}

	const Token& advance() {
		if (!isAtEnd()) {
			return *(it++);
		}
		return *it;
	}

	bool isAtEnd() const {
		return it == tokens.end() || it->getType() == TokenType::END_OF_FILE;
	}

	vector<Token> tokens;
	vector<Token>::const_iterator it;
};