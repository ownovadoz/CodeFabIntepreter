#pragma once

#include "Expression.h"
#include "../../Node.h"
#include "../Tokenizer/Token.h"

#include <memory>
#include <vector>

using std::shared_ptr;
using std::vector;

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

	shared_ptr<Node> parse();

private:
	shared_ptr<Node> parseVarStmt();
	shared_ptr<Expression> parseExpression();
	void nextToken();
	bool expectPeek(TokenType type);
	bool isLiteralOperand(TokenType type) const;
	bool isBinaryOperator(TokenType type) const;

	vector<Token> tokens;
	size_t current_idx = 0;
	Token current_token;
	Token peek_token;
};