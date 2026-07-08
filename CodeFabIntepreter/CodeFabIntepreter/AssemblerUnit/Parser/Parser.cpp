#include "Parser.h"
#include "Statement.h"
#include "Expression.h"

#include <vector>
#include <stdexcept>

using std::vector;
using std::exception;

// TODO: can I change return type as const Statement*?
Statement* Parser::parse(const vector<Token>& tokens) {
	if (tokens.empty()) return nullptr;
	init(tokens);
	return parseStatement();
}

Statement* Parser::parseStatement() {
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
	case TokenType::END_OF_FILE:
		break;	// TODO: what should I return?
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
		throw exception();
	}
	return nullptr;
}

Statement* Parser::parseIfStmt() {
	return nullptr;
}

Statement* Parser::parseBlockStmt() {
	return nullptr;
}

Statement* Parser::parseVarDeclareStmt() {
	if (advance().getType() != TokenType::VAR) throw exception();
	if (peek().getType() != TokenType::IDENTIFIER) throw exception();

	VarDeclareStmt* stmt = new VarDeclareStmt{ advance() };

	if (advance().getType() != TokenType::EQUAL) throw exception();

	Expression* expr = parseExpression();

	if (expr == nullptr) {
		delete stmt;
		throw exception();
	}

	stmt->setExpression(expr);

	return stmt;
}

Statement* Parser::parsePrintStmt() {
	return nullptr;
}

Statement* Parser::parseForStmt() {
	return nullptr;
}

Statement* Parser::parseExpressionStmt() {
	return nullptr;
}

Expression* Parser::parseExpression() {
	// now, only support single number case
	if (peek().getType() == TokenType::NUMBER) {
		Expression* expr = parseNumberExpr();
		if (advance().getType() == TokenType::SEMICOLON && peek().getType() == TokenType::END_OF_FILE) {
			return expr;
		}
	}
	return nullptr;
}

Expression* Parser::parseNumberExpr() {
	return new LiteralExpr(advance());
}
