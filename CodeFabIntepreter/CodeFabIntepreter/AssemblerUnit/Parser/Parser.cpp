#include "Parser.h"
#include "Statement.h"

#include <memory>

using std::shared_ptr;
using std::make_shared;

Node* Parser::parse()
{
	switch (tokens.front().type) {
	case TokenType::VAR:
		return parseVarStmt();
	}

	return nullptr;
}

Node* Parser::parseVarStmt() {
	auto stmt = new VarDeclareStmt();

	if (!expectPeek(TokenType::IDENTIFIER)) {
		return nullptr;	// throw
	}

	stmt->setName(current_token);

	if (expectPeek(TokenType::EQUAL)) {
		auto initializer = parseExpression();
		if (!initializer) {
			return nullptr;	// throw
		}

		stmt->setInitializer(initializer);
	}

	if (!expectPeek(TokenType::SEMICOLON)) {
		return nullptr;	// throw
	}

	return stmt;
}

shared_ptr<Expression> Parser::parseExpression() {
	if (!(expectPeek(TokenType::NUMBER) || expectPeek(TokenType::STRING))) {
		return nullptr;	// throw
	}

	shared_ptr<Expression> left = make_shared<LiteralExpr>(current_token);

	if (isBinaryOperator(peek_token.type)) {
		nextToken();
		Token op = current_token;

		if (!(expectPeek(TokenType::NUMBER) || expectPeek(TokenType::STRING))) {
			return nullptr;	// throw
		}

		shared_ptr<Expression> right = make_shared<LiteralExpr>(current_token);
		left = make_shared<BinaryExpr>(left, op, right);
	}

	return left;
}

bool Parser::isBinaryOperator(TokenType type) {
	return type == TokenType::PLUS || type == TokenType::MINUS
		|| type == TokenType::STAR || type == TokenType::SLASH;
}

void Parser::nextToken() {
	current_token = peek_token;
	current_idx++;
	if (current_idx + 1 < tokens.size()) {
		peek_token = tokens[current_idx + 1];
	}
	else {
		peek_token = Token{ TokenType::END_OF_FILE, "" };
	}
}

bool Parser::expectPeek(TokenType type) {
	if (peek_token.type == type) {
		nextToken();
		return true;
	}
	else {
		return false;
	}
}
