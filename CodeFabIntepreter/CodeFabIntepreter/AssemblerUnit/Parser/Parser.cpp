#include "Parser.h"
#include "Statement.h"
#include "Expression.h"
#include "../../CodeFabException.h"

#include <memory>
#include <vector>

using std::make_unique;
using std::move;
using std::string;
using std::unique_ptr;
using std::vector;

Token Parser::consume(TokenType type, const string& message) {
	if (check(type)) return advance();
	throw CodeFabException(peek(), message);
}

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
	case TokenType::ARRAY:
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
	case TokenType::LEFT_BRACKET:
	case TokenType::RIGHT_BRACKET:
		throw CodeFabException(token, "Unexpected token.");
	}
	return nullptr;
}

unique_ptr<Statement> Parser::parseIfStmt() {
	advance();

	consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");

	unique_ptr<Expression> condition = parseExpression();

	consume(TokenType::RIGHT_PAREN, "Expect ')' after if condition.");

	unique_ptr<Statement> then_branch = parseStatement();
	unique_ptr<Statement> else_branch = nullptr;

	if (peek().getType() == TokenType::ELSE) {
		advance();
		else_branch = parseStatement();
	}

	return make_unique<IfStmt>(move(condition), move(then_branch), move(else_branch));
}

unique_ptr<Statement> Parser::parseBlockStmt() {
	advance();

	auto block = make_unique<BlockStmt>();
	while (!isAtEnd() && peek().getType() != TokenType::RIGHT_BRACE) {
		block->addStatement(parseStatement());
	}

	consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");

	return block;
}

unique_ptr<Statement> Parser::parseVarDeclareStmt() {
	consume(TokenType::VAR, "Expect 'var'.");

	auto stmt = make_unique<VarDeclareStmt>(consume(TokenType::IDENTIFIER, "Expect variable name."));

	consume(TokenType::EQUAL, "Expect '=' after variable name.");

	unique_ptr<Expression> expr = parseExpression();

	consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");

	stmt->setExpression(move(expr));

	return stmt;
}

unique_ptr<Statement> Parser::parsePrintStmt() {
	advance();

	unique_ptr<Expression> expr = parseExpression();

	consume(TokenType::SEMICOLON, "Expect ';' after value.");

	return make_unique<PrintStmt>(move(expr));
}

unique_ptr<Statement> Parser::parseForStmt() {
	advance();

	consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");

	unique_ptr<Statement> raw_init = parseVarDeclareStmt();
	unique_ptr<VarDeclareStmt> init(static_cast<VarDeclareStmt*>(raw_init.release()));

	unique_ptr<Expression> condition = parseExpression();

	consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");

	unique_ptr<Expression> increment = parseExpression();

	consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");

	unique_ptr<Statement> body = parseStatement();

	return make_unique<ForStmt>(move(init), move(condition), move(increment), move(body));
}

unique_ptr<Statement> Parser::parseExpressionStmt() {
	unique_ptr<Expression> expr = parseExpression();

	consume(TokenType::SEMICOLON, "Expect ';' after expression.");

	return make_unique<ExpressionStmt>(move(expr));
}

unique_ptr<Expression> Parser::parseExpression() {
	return parseAssignExpr();
}

unique_ptr<Expression> Parser::parseAssignExpr() {
	unique_ptr<Expression> left = parseLogicOr();

	if (peek().getType() == TokenType::EQUAL) {
		Token equals = advance();

		VariableExpr* variable = dynamic_cast<VariableExpr*>(left.get());
		IndexExpr* index_expr = (variable != nullptr) ? nullptr : dynamic_cast<IndexExpr*>(left.get());

		if (variable == nullptr && index_expr == nullptr) throw CodeFabException(equals, "Invalid assignment target.");

		unique_ptr<Expression> value = parseAssignExpr();

		if (variable != nullptr) {
			return make_unique<AssignExpr>(variable->getToken(), move(value));
		}

		return make_unique<IndexSetExpr>(index_expr->releaseArray(), index_expr->releaseIndex(), move(value));
	}

	return left;
}

unique_ptr<Expression> Parser::parseLogicOr() {
	return parseLeftAssocExpr(&Parser::parseLogicAnd, { TokenType::OR }, [](unique_ptr<Expression> left, const Token& op, unique_ptr<Expression> right) -> unique_ptr<Expression> {
		return make_unique<LogicalExpr>(move(left), op, move(right));
	});
}

unique_ptr<Expression> Parser::parseLogicAnd() {
	return parseLeftAssocExpr(&Parser::parseEquality, { TokenType::AND }, [](unique_ptr<Expression> left, const Token& op, unique_ptr<Expression> right) -> unique_ptr<Expression> {
		return make_unique<LogicalExpr>(move(left), op, move(right));
	});
}

unique_ptr<Expression> Parser::parseEquality() {
	return parseLeftAssocExpr(&Parser::parseComparison, { TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL }, [](unique_ptr<Expression> left, const Token& op, unique_ptr<Expression> right) -> unique_ptr<Expression> {
		return make_unique<BinaryExpr>(move(left), op, move(right));
	});
}

unique_ptr<Expression> Parser::parseComparison() {
	return parseLeftAssocExpr(&Parser::parseTerm, { TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL }, [](unique_ptr<Expression> left, const Token& op, unique_ptr<Expression> right) -> unique_ptr<Expression> {
		return make_unique<BinaryExpr>(move(left), op, move(right));
	});
}

unique_ptr<Expression> Parser::parseTerm() {
	return parseLeftAssocExpr(&Parser::parseFactor, { TokenType::PLUS, TokenType::MINUS }, [](unique_ptr<Expression> left, const Token& op, unique_ptr<Expression> right) -> unique_ptr<Expression> {
		return make_unique<BinaryExpr>(move(left), op, move(right));
	});
}

unique_ptr<Expression> Parser::parseFactor() {
	return parseLeftAssocExpr(&Parser::parseUnaryExpr, { TokenType::STAR, TokenType::SLASH }, [](unique_ptr<Expression> left, const Token& op, unique_ptr<Expression> right) -> unique_ptr<Expression> {
		return make_unique<BinaryExpr>(move(left), op, move(right));
	});
}

unique_ptr<Expression> Parser::parseLeftAssocExpr(unique_ptr<Expression> (Parser::* parseOperand)(), initializer_list<TokenType> operators, ExprFactory makeExpr) {
	unique_ptr<Expression> expr = (this->*parseOperand)();

	while (checkAny(operators)) {
		Token op = advance();
		unique_ptr<Expression> right = (this->*parseOperand)();
		expr = makeExpr(move(expr), op, move(right));
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
	return parsePostfixExpr();
}

unique_ptr<Expression> Parser::parsePostfixExpr() {
	unique_ptr<Expression> expr = parsePrimaryExpr();

	while (check(TokenType::LEFT_BRACKET)) {
		advance();

		unique_ptr<Expression> index = parseExpression();

		const LiteralExpr* literal = dynamic_cast<const LiteralExpr*>(index.get());
		if (literal != nullptr && literal->getToken().getType() != TokenType::NUMBER) {
			throw CodeFabException(literal->getToken(), "Array index must be a number.");
		}

		consume(TokenType::RIGHT_BRACKET, "Expect ']' after index.");

		expr = make_unique<IndexExpr>(move(expr), move(index));
	}

	return expr;
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
	case TokenType::ARRAY:
		return parseArrayExpr();
	case TokenType::LEFT_PAREN: {
		advance();
		unique_ptr<Expression> expr = parseExpression();

		consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");

		return make_unique<GroupingExpr>(move(expr));
	}
	default:
		throw CodeFabException(peek(), "Expect expression.");
	}
}

unique_ptr<Expression> Parser::parseArrayExpr() {
	advance();

	consume(TokenType::LEFT_PAREN, "Expect '(' after 'Array'.");

	unique_ptr<Expression> size = parseExpression();

	const LiteralExpr* literal = dynamic_cast<const LiteralExpr*>(size.get());
	if (literal != nullptr && literal->getToken().getType() != TokenType::NUMBER) {
		throw CodeFabException(literal->getToken(), "Array size must be a number.");
	}

	consume(TokenType::RIGHT_PAREN, "Expect ')' after array size.");

	return make_unique<ArrayExpr>(move(size));
}
