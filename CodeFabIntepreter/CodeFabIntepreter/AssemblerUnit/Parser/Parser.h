#pragma once

#include "../../Node.h"
#include "../Tokenizer/Token.h"
#include "Expression.h"

#include <vector>
#include <memory>

using std::vector;
using std::shared_ptr;

class Parser
{
public:
	explicit Parser(vector<Token>& tokens) : tokens{ tokens } {
		current_idx = 0;
		if (!this->tokens.empty()) {
			current_token = this->tokens[0];
			if (this->tokens.size() > 1) {
				peek_token = this->tokens[1];
			}
		}
	}

	Node* parse();

private:
	Node* parseVarStmt();
	shared_ptr<Expression> parseExpression();
	void nextToken();
	bool expectPeek(TokenType type);
	bool isBinaryOperator(TokenType type);

	vector<Token> tokens;
	size_t current_idx = 0;
	Token current_token;
	Token peek_token;
};