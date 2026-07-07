#include "Parser.h"
#include "Expression.h"
#include "Statement.h"

#include <memory>

using std::shared_ptr;
using std::make_shared;

shared_ptr<Node> Parser::parse()
{
	switch (tokens.front().getType()) {
	case TokenType::VAR:
		return parseVarStmt();
	}

	return nullptr;
}

shared_ptr<Node> Parser::parseVarStmt() {
	if (!expectPeek(TokenType::IDENTIFIER)) return nullptr;	// throw
	Token name = current_token;

	shared_ptr<Expression> initializer;
	if (expectPeek(TokenType::EQUAL)) {
		initializer = parseExpression();
		if (!initializer) return nullptr;	// throw
	}

	if (!expectPeek(TokenType::SEMICOLON)) return nullptr;	// throw

	auto stmt = make_shared<VarDeclareStmt>();
	stmt->setName(name);
	stmt->setInitializer(initializer);
	return stmt;
}

shared_ptr<Expression> Parser::parseExpression() {
	if (!isLiteralOperand(peek_token.getType())) return nullptr;	// throw
	nextToken();

	shared_ptr<Expression> left = make_shared<LiteralExpr>(current_token);

	if (isBinaryOperator(peek_token.getType())) {
		nextToken();
		Token op = current_token;

		if (!isLiteralOperand(peek_token.getType())) return nullptr;	// throw
		nextToken();

		shared_ptr<Expression> right = make_shared<LiteralExpr>(current_token);
		left = make_shared<BinaryExpr>(left, op, right);
	}

	return left;
}

bool Parser::isLiteralOperand(TokenType type) const {
	return type == TokenType::NUMBER || type == TokenType::STRING;
}

bool Parser::isBinaryOperator(TokenType type) const {
	return type == TokenType::PLUS || type == TokenType::MINUS
		|| type == TokenType::STAR || type == TokenType::SLASH;
}

void Parser::nextToken() {
	current_token = peek_token;
	current_idx++;
	if (current_idx + 1 < tokens.size()) peek_token = tokens[current_idx + 1];
	else peek_token = Token{ TokenType::END_OF_FILE, "" };
}

bool Parser::expectPeek(TokenType type) {
	if (peek_token.getType() != type) return false;

	nextToken();
	return true;
}
