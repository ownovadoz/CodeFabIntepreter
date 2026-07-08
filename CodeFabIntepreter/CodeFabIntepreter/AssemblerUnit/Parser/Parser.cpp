#include "Parser.h"
#include "Statement.h"
#include "Expression.h"
#include "../../CodeFabException.h"

#include <memory>
#include <vector>

using std::make_unique;
using std::move;
using std::unique_ptr;
using std::vector;

vector<unique_ptr<Statement>> Parser::parse(const vector<Token>& tokens) {
	if (tokens.empty()) return {};
	init(tokens);

	vector<unique_ptr<Statement>> program;
	while (!isAtEnd()) {
		program.push_back(parseStatement());
	}
	return program;
}

unique_ptr<Statement> Parser::parseStatement() {
	const Token& token = peek();
	switch (token.getType()) {
	case TokenType::IF:
		return parseIfStmt();
	case TokenType::LEFT_BRACE:
		return parseBlockStmt();
	case TokenType::VAR:
		return parseVarDeclareStmt();
	case TokenType::PRINT:
		return parsePrintStmt();
	case TokenType::FOR:
		return parseForStmt();
	case TokenType::LEFT_PAREN:
	case TokenType::BANG:
	case TokenType::IDENTIFIER:
	case TokenType::STRING:
	case TokenType::NUMBER:
	case TokenType::FALSE:
	case TokenType::TRUE:
		return parseExpressionStmt();
	case TokenType::SEMICOLON:
		advance();
		return nullptr;
	case TokenType::END_OF_FILE:
		return nullptr;
	case TokenType::RIGHT_PAREN:
	case TokenType::RIGHT_BRACE:
	case TokenType::PLUS:
	case TokenType::MINUS:
	case TokenType::STAR:
	case TokenType::SLASH:
	case TokenType::BANG_EQUAL:
	case TokenType::EQUAL:
	case TokenType::EQUAL_EQUAL:
	case TokenType::GREATER:
	case TokenType::GREATER_EQUAL:
	case TokenType::LESS:
	case TokenType::LESS_EQUAL:
	case TokenType::AND:
	case TokenType::OR:
	case TokenType::ELSE:
		throw CodeFabException(token, "Unexpected token.");
	}
	return nullptr;
}

unique_ptr<Statement> Parser::parseIfStmt() {
	throw CodeFabException(peek(), "'if' statements are not implemented yet.");
}

unique_ptr<Statement> Parser::parseBlockStmt() {
	advance();

	auto block = make_unique<BlockStmt>();
	while (!isAtEnd() && peek().getType() != TokenType::RIGHT_BRACE) {
		block->addStatement(parseStatement());
	}

	if (peek().getType() != TokenType::RIGHT_BRACE) throw CodeFabException(peek(), "Expect '}' after block.");
	advance();

	return block;
}

unique_ptr<Statement> Parser::parseVarDeclareStmt() {
	Token var_token = advance();
	if (var_token.getType() != TokenType::VAR) throw CodeFabException(var_token, "Expect 'var'.");
	if (peek().getType() != TokenType::IDENTIFIER) throw CodeFabException(peek(), "Expect variable name.");

	auto stmt = make_unique<VarDeclareStmt>(advance());

	Token equal_token = advance();
	if (equal_token.getType() != TokenType::EQUAL) throw CodeFabException(equal_token, "Expect '=' after variable name.");

	unique_ptr<Expression> expr = parseExpression();

	Token after_expr_token = advance();
	if (after_expr_token.getType() != TokenType::SEMICOLON) {
		throw CodeFabException(after_expr_token, "Expect ';' after variable declaration.");
	}

	stmt->setExpression(move(expr));

	return stmt;
}

unique_ptr<Statement> Parser::parsePrintStmt() {
	throw CodeFabException(peek(), "'print' statements are not implemented yet.");
}

unique_ptr<Statement> Parser::parseForStmt() {
	throw CodeFabException(peek(), "'for' statements are not implemented yet.");
}

unique_ptr<Statement> Parser::parseExpressionStmt() {
	throw CodeFabException(peek(), "Expression statements are not implemented yet.");
}

unique_ptr<Expression> Parser::parseExpression() {
	return parseAssignExpr();
}

unique_ptr<Expression> Parser::parseAssignExpr() {
	unique_ptr<Expression> left = parseLogicOr();

	if (peek().getType() == TokenType::EQUAL) {
		Token equals = advance();

		VariableExpr* variable = dynamic_cast<VariableExpr*>(left.get());
		if (variable == nullptr) throw CodeFabException(equals, "Invalid assignment target.");

		Token identifier = variable->getToken();
		unique_ptr<Expression> value = parseAssignExpr();

		return make_unique<AssignExpr>(identifier, move(value));
	}

	return left;
}

unique_ptr<Expression> Parser::parseLogicOr() {
	unique_ptr<Expression> expr = parseLogicAnd();

	while (peek().getType() == TokenType::OR) {
		Token op = advance();
		unique_ptr<Expression> right = parseLogicAnd();
		expr = make_unique<LogicalExpr>(move(expr), op, move(right));
	}

	return expr;
}

unique_ptr<Expression> Parser::parseLogicAnd() {
	unique_ptr<Expression> expr = parseEquality();

	while (peek().getType() == TokenType::AND) {
		Token op = advance();
		unique_ptr<Expression> right = parseEquality();
		expr = make_unique<LogicalExpr>(move(expr), op, move(right));
	}

	return expr;
}

unique_ptr<Expression> Parser::parseEquality() {
	unique_ptr<Expression> expr = parseComparison();

	while (peek().getType() == TokenType::BANG_EQUAL || peek().getType() == TokenType::EQUAL_EQUAL) {
		Token op = advance();
		unique_ptr<Expression> right = parseComparison();
		expr = make_unique<BinaryExpr>(move(expr), op, move(right));
	}

	return expr;
}

unique_ptr<Expression> Parser::parseComparison() {
	unique_ptr<Expression> expr = parseTerm();

	while (peek().getType() == TokenType::GREATER || peek().getType() == TokenType::GREATER_EQUAL
		|| peek().getType() == TokenType::LESS || peek().getType() == TokenType::LESS_EQUAL) {
		Token op = advance();
		unique_ptr<Expression> right = parseTerm();
		expr = make_unique<BinaryExpr>(move(expr), op, move(right));
	}

	return expr;
}

unique_ptr<Expression> Parser::parseTerm() {
	unique_ptr<Expression> expr = parseFactor();

	while (peek().getType() == TokenType::PLUS || peek().getType() == TokenType::MINUS) {
		Token op = advance();
		unique_ptr<Expression> right = parseFactor();
		expr = make_unique<BinaryExpr>(move(expr), op, move(right));
	}

	return expr;
}

unique_ptr<Expression> Parser::parseFactor() {
	unique_ptr<Expression> expr = parseUnaryExpr();

	while (peek().getType() == TokenType::STAR || peek().getType() == TokenType::SLASH) {
		Token op = advance();
		unique_ptr<Expression> right = parseUnaryExpr();
		expr = make_unique<BinaryExpr>(move(expr), op, move(right));
	}

	return expr;
}

unique_ptr<Expression> Parser::parseUnaryExpr() {
	TokenType type = peek().getType();
	if (type == TokenType::MINUS || type == TokenType::BANG) {
		Token op = advance();
		unique_ptr<Expression> operand = parseUnaryExpr();
		return make_unique<UnaryExpr>(op, move(operand));
	}
	return parsePrimaryExpr();
}

unique_ptr<Expression> Parser::parsePrimaryExpr() {
	switch (peek().getType()) {
	case TokenType::NUMBER:
	case TokenType::STRING:
	case TokenType::TRUE:
	case TokenType::FALSE:
		return make_unique<LiteralExpr>(advance());
	case TokenType::IDENTIFIER:
		return make_unique<VariableExpr>(advance());
	case TokenType::LEFT_PAREN: {
		advance();
		unique_ptr<Expression> expr = parseExpression();

		Token close_paren = advance();
		if (close_paren.getType() != TokenType::RIGHT_PAREN) throw CodeFabException(close_paren, "Expect ')' after expression.");

		return make_unique<GroupingExpr>(move(expr));
	}
	default:
		throw CodeFabException(peek(), "Expect expression.");
	}
}
