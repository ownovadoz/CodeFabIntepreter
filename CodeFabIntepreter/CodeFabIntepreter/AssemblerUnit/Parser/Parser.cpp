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
	case TokenType::FUNC:
		return parseFunctionStmt();
	case TokenType::RETURN:
		return parseReturnStmt();
	case TokenType::CLASS:
		return parseClassStmt();
	case TokenType::IMPORT:
		return parseImportStmt();
	case TokenType::LEFT_PAREN:
	case TokenType::BANG:
	case TokenType::IDENTIFIER:
	case TokenType::STRING:
	case TokenType::NUMBER:
	case TokenType::FALSE:
	case TokenType::TRUE:
	case TokenType::THIS:
	case TokenType::SUPER:
	case TokenType::ARRAY:
		return parseExpressionStmt();
	case TokenType::SEMICOLON:
		advance();
		return nullptr;
	case TokenType::END_OF_FILE:
		return nullptr;
	case TokenType::RIGHT_PAREN:
	case TokenType::RIGHT_BRACE:
	case TokenType::LEFT_BRACKET:
	case TokenType::RIGHT_BRACKET:
	case TokenType::COMMA:
	case TokenType::DOT:
	case TokenType::COLON:
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
	case TokenType::INSTANCEOF:
	case TokenType::ALIAS:
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

	return parseBlock();
}

unique_ptr<BlockStmt> Parser::parseBlock() {
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

unique_ptr<Statement> Parser::parseFunctionStmt() {
	advance();

	Token name = consume(TokenType::IDENTIFIER, "Expect function name.");

	return finishFunctionDecl(name);
}

// `(params) { body }` 부분을 읽어 FunctionStmt를 만든다. `Func name` 선언문과
// 클래스 메서드 선언(이름 뒤에 바로 괄호가 온다) 양쪽에서 공유한다.
unique_ptr<FunctionStmt> Parser::finishFunctionDecl(const Token& name) {
	consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");
	vector<Token> params = parseParameters();
	consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");

	consume(TokenType::LEFT_BRACE, "Expect '{' before function body.");
	unique_ptr<BlockStmt> body = parseBlock();

	return make_unique<FunctionStmt>(name, move(params), move(body));
}

vector<Token> Parser::parseParameters() {
	vector<Token> params;

	if (!check(TokenType::RIGHT_PAREN)) {
		params.push_back(consume(TokenType::IDENTIFIER, "Expect parameter name."));

		while (check(TokenType::COMMA)) {
			advance();
			params.push_back(consume(TokenType::IDENTIFIER, "Expect parameter name."));
		}
	}

	return params;
}

unique_ptr<Statement> Parser::parseReturnStmt() {
	Token keyword = advance();

	unique_ptr<Expression> value = nullptr;
	if (!check(TokenType::SEMICOLON)) {
		value = parseExpression();
	}

	consume(TokenType::SEMICOLON, "Expect ';' after return value.");

	return make_unique<ReturnStmt>(keyword, move(value));
}

unique_ptr<Statement> Parser::parseClassStmt() {
	advance();

	Token name = consume(TokenType::IDENTIFIER, "Expect class name.");

	unique_ptr<VariableExpr> superclass = nullptr;
	if (check(TokenType::COLON)) {
		advance();
		superclass = make_unique<VariableExpr>(consume(TokenType::IDENTIFIER, "Expect superclass name."));
	}

	consume(TokenType::LEFT_BRACE, "Expect '{' before class body.");

	vector<unique_ptr<FunctionStmt>> methods;
	while (!isAtEnd() && !check(TokenType::RIGHT_BRACE)) {
		Token method_name = consume(TokenType::IDENTIFIER, "Expect method name.");
		methods.push_back(finishFunctionDecl(method_name));
	}

	consume(TokenType::RIGHT_BRACE, "Expect '}' after class body.");

	return make_unique<ClassStmt>(name, move(superclass), move(methods));
}

unique_ptr<Statement> Parser::parseImportStmt() {
	advance();

	Token path = consume(TokenType::STRING, "Expect file path string after 'import'.");
	consume(TokenType::ALIAS, "Expect 'alias' after import path.");
	Token alias = consume(TokenType::IDENTIFIER, "Expect alias name after 'alias'.");
	consume(TokenType::SEMICOLON, "Expect ';' after import statement.");

	return make_unique<ImportStmt>(path, alias);
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
		unique_ptr<Expression> value = parseAssignExpr();

		if (VariableExpr* variable = dynamic_cast<VariableExpr*>(left.get())) {
			return make_unique<AssignExpr>(variable->getToken(), move(value));
		}

		if (GetExpr* get = dynamic_cast<GetExpr*>(left.get())) {
			return make_unique<SetExpr>(get->releaseObject(), get->getName(), move(value));
		}

		if (IndexExpr* index_expr = dynamic_cast<IndexExpr*>(left.get())) {
			return make_unique<IndexSetExpr>(index_expr->releaseArray(), index_expr->releaseIndex(), move(value));
		}

		throw CodeFabException(equals, "Invalid assignment target.");
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
	return parseLeftAssocExpr(&Parser::parseInstanceOf, { TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL }, [](unique_ptr<Expression> left, const Token& op, unique_ptr<Expression> right) -> unique_ptr<Expression> {
		return make_unique<BinaryExpr>(move(left), op, move(right));
	});
}

unique_ptr<Expression> Parser::parseInstanceOf() {
	unique_ptr<Expression> expr = parseComparison();

	while (check(TokenType::INSTANCEOF)) {
		Token keyword = advance();
		Token class_name = consume(TokenType::IDENTIFIER, "Expect class name after 'instanceof'.");
		expr = make_unique<InstanceOfExpr>(move(expr), keyword, class_name);
	}

	return expr;
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
	return parseCallExpr();
}

unique_ptr<Expression> Parser::parseCallExpr() {
	unique_ptr<Expression> expr = parsePrimaryExpr();

	while (true) {
		if (check(TokenType::LEFT_PAREN)) {
			expr = finishCallExpr(move(expr));
		}
		else if (check(TokenType::LEFT_BRACKET)) {
			expr = finishIndexExpr(move(expr));
		}
		else if (check(TokenType::DOT)) {
			advance();
			Token name = consume(TokenType::IDENTIFIER, "Expect property name after '.'.");
			expr = make_unique<GetExpr>(move(expr), name);
		}
		else {
			break;
		}
	}

	return expr;
}

unique_ptr<Expression> Parser::finishCallExpr(unique_ptr<Expression> callee) {
	advance();

	vector<unique_ptr<Expression>> arguments;
	if (!check(TokenType::RIGHT_PAREN)) {
		arguments.push_back(parseExpression());

		while (check(TokenType::COMMA)) {
			advance();
			arguments.push_back(parseExpression());
		}
	}

	Token paren = consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");

	return make_unique<CallExpr>(move(callee), paren, move(arguments));
}

unique_ptr<Expression> Parser::finishIndexExpr(unique_ptr<Expression> array) {
	advance();

	unique_ptr<Expression> index = parseExpression();
	rejectNonNumericLiteral(index.get(), "배열 인덱스는 숫자여야 합니다.");

	consume(TokenType::RIGHT_BRACKET, "Expect ']' after index.");

	return make_unique<IndexExpr>(move(array), move(index));
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
	case TokenType::THIS:
		return make_unique<ThisExpr>(advance());
	case TokenType::SUPER: {
		Token keyword = advance();
		consume(TokenType::DOT, "Expect '.' after 'Super'.");
		Token method = consume(TokenType::IDENTIFIER, "Expect superclass method name.");
		return make_unique<SuperExpr>(keyword, method);
	}
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
	rejectNonNumericLiteral(size.get(), "배열 크기는 숫자여야 합니다.");

	consume(TokenType::RIGHT_PAREN, "Expect ')' after array size.");

	return make_unique<ArrayExpr>(move(size));
}

void Parser::rejectNonNumericLiteral(const Expression* expr, const string& message) {
	const LiteralExpr* literal = dynamic_cast<const LiteralExpr*>(expr);
	if (literal != nullptr && literal->getToken().getType() != TokenType::NUMBER) {
		throw CodeFabException(literal->getToken(), message);
	}
}
