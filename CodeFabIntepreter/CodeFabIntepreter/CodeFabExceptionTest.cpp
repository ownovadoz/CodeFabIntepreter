#include "CodeFabException.h"

#include <gmock/gmock.h>

TEST(CodeFabExceptionTest, LineConstructorFormatsMessageWithLineNumber) {
	CodeFabException error(7, "unexpected character");

	EXPECT_EQ(error.getLine(), 7);
	EXPECT_EQ(error.getMessage(), "unexpected character");
	EXPECT_STREQ(error.what(), "[line 7] Error: unexpected character");
}

TEST(CodeFabExceptionTest, IsCatchableAsStdException) {
	try {
		throw CodeFabException(1, "boom");
	}
	catch (const std::exception& e) {
		EXPECT_STREQ(e.what(), "[line 1] Error: boom");
		return;
	}

	FAIL() << "CodeFabException should be catchable as std::exception";
}

TEST(CodeFabExceptionTest, TokenConstructorFormatsMessageWithLexeme) {
	Token token(TokenType::SEMICOLON, ";", std::monostate{}, 12);
	CodeFabException error(token, "expected expression");

	EXPECT_EQ(error.getLine(), 12);
	EXPECT_EQ(error.getMessage(), "expected expression");
	EXPECT_STREQ(error.what(), "[line 12] Type : SEMICOLON, Lexeme : ;, literal : nil, message : expected expression");
}