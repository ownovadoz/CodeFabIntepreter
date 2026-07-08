#include "Lexer.h"

#include <gmock/gmock.h>
#include <vector>

using std::vector;
using namespace testing;

class LexerTestFixture : public ::testing::Test
{
protected:
    vector<Token> scan(const string& source)
    {
        Lexer lexer(source);
        return lexer.scanTokens();
    }
};

TEST_F(LexerTestFixture, SingleCharTokens)
{
    auto tokens = scan("( ) { } ; + - * = ! > <");

    EXPECT_EQ(tokens[0].getType(),  TokenType::LEFT_PAREN);
    EXPECT_EQ(tokens[1].getType(),  TokenType::RIGHT_PAREN);
    EXPECT_EQ(tokens[2].getType(),  TokenType::LEFT_BRACE);
    EXPECT_EQ(tokens[3].getType(),  TokenType::RIGHT_BRACE);
    EXPECT_EQ(tokens[4].getType(),  TokenType::SEMICOLON);
    EXPECT_EQ(tokens[5].getType(),  TokenType::PLUS);
    EXPECT_EQ(tokens[6].getType(),  TokenType::MINUS);
    EXPECT_EQ(tokens[7].getType(),  TokenType::STAR);
    EXPECT_EQ(tokens[8].getType(),  TokenType::EQUAL);
    EXPECT_EQ(tokens[9].getType(),  TokenType::BANG);
    EXPECT_EQ(tokens[10].getType(), TokenType::GREATER);
    EXPECT_EQ(tokens[11].getType(), TokenType::LESS);
    EXPECT_EQ(tokens[12].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTestFixture, BracketTokens)
{
    auto tokens = scan("[ ]");

    EXPECT_EQ(tokens[0].getType(), TokenType::LEFT_BRACKET);
    EXPECT_EQ(tokens[1].getType(), TokenType::RIGHT_BRACKET);
    EXPECT_EQ(tokens[2].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTestFixture, SlashToken)
{
    auto tokens = scan("/");

    EXPECT_EQ(tokens[0].getType(), TokenType::SLASH);
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTestFixture, CommentIsIgnored)
{
    auto tokens = scan("// this is a comment\n+");

    EXPECT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].getType(), TokenType::PLUS);
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTestFixture, WhitespaceIsIgnoredAndNewlineIncrementsLine)
{
    auto tokens = scan("  +  \t\n-  ");

    EXPECT_EQ(tokens.size(), 3u);
    EXPECT_EQ(tokens[0].getType(), TokenType::PLUS);
    EXPECT_EQ(tokens[0].getLine(), 1);
    EXPECT_EQ(tokens[1].getType(), TokenType::MINUS);
    EXPECT_EQ(tokens[1].getLine(), 2);
}

TEST_F(LexerTestFixture, EmptySourceReturnsOnlyEOF)
{
    auto tokens = scan("");

    EXPECT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTestFixture, TokenMetaIsCorrect)
{
    auto tokens = scan("+");

    EXPECT_EQ(tokens[0].getLexeme(), "+");
    EXPECT_EQ(tokens[0].getLine(), 1);
}

TEST_F(LexerTestFixture, MultiCharTokens)
{
    auto tokens = scan("== != <= >=");

    EXPECT_EQ(tokens[0].getType(), TokenType::EQUAL_EQUAL);
    EXPECT_EQ(tokens[1].getType(), TokenType::BANG_EQUAL);
    EXPECT_EQ(tokens[2].getType(), TokenType::LESS_EQUAL);
    EXPECT_EQ(tokens[3].getType(), TokenType::GREATER_EQUAL);
    EXPECT_EQ(tokens[4].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTestFixture, MultiCharTokenLexemeIsCorrect)
{
    auto tokens = scan("==");

    EXPECT_EQ(tokens[0].getLexeme(), "==");
}

TEST_F(LexerTestFixture, SingleCharNotConsumedByMultiChar)
{
    auto tokens = scan("= ! < >");

    EXPECT_EQ(tokens[0].getType(), TokenType::EQUAL);
    EXPECT_EQ(tokens[1].getType(), TokenType::BANG);
    EXPECT_EQ(tokens[2].getType(), TokenType::LESS);
    EXPECT_EQ(tokens[3].getType(), TokenType::GREATER);
    EXPECT_EQ(tokens[4].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTestFixture, StringLiteral)
{
    auto tokens = scan("\"hello\"");

    EXPECT_EQ(tokens[0].getType(), TokenType::STRING);
    EXPECT_EQ(std::get<string>(tokens[0].getLiteral()), "hello");
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTestFixture, StringLiteralLexemeIncludesQuotes)
{
    auto tokens = scan("\"hi\"");

    EXPECT_EQ(tokens[0].getLexeme(), "\"hi\"");
}

TEST_F(LexerTestFixture, IntegerLiteral)
{
    auto tokens = scan("123");

    EXPECT_EQ(tokens[0].getType(), TokenType::NUMBER);
    EXPECT_DOUBLE_EQ(std::get<double>(tokens[0].getLiteral()), 123.0);
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTestFixture, FloatLiteral)
{
    auto tokens = scan("3.14");

    EXPECT_EQ(tokens[0].getType(), TokenType::NUMBER);
    EXPECT_DOUBLE_EQ(std::get<double>(tokens[0].getLiteral()), 3.14);
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTestFixture, MultipleLiterals)
{
    auto tokens = scan("123 \"abc\" 4.5");

    EXPECT_EQ(tokens[0].getType(), TokenType::NUMBER);
    EXPECT_DOUBLE_EQ(std::get<double>(tokens[0].getLiteral()), 123.0);
    EXPECT_EQ(tokens[1].getType(), TokenType::STRING);
    EXPECT_EQ(std::get<string>(tokens[1].getLiteral()), "abc");
    EXPECT_EQ(tokens[2].getType(), TokenType::NUMBER);
    EXPECT_DOUBLE_EQ(std::get<double>(tokens[2].getLiteral()), 4.5);
    EXPECT_EQ(tokens[3].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTestFixture, IdentifierToken)
{
    auto tokens = scan("abc");

    EXPECT_EQ(tokens[0].getType(), TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[0].getLexeme(), "abc");
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTestFixture, KeywordVar)
{
    auto tokens = scan("var");

    EXPECT_EQ(tokens[0].getType(), TokenType::VAR);
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTestFixture, KeywordArray)
{
    auto tokens = scan("Array");

    EXPECT_EQ(tokens[0].getType(), TokenType::ARRAY);
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTestFixture, IdentifierStartsWithArrayIsNotKeyword)
{
    auto tokens = scan("ArrayList");

    EXPECT_EQ(tokens[0].getType(), TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTestFixture, KeywordIf)
{
    auto tokens = scan("if");

    EXPECT_EQ(tokens[0].getType(), TokenType::IF);
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTestFixture, KeywordTrue)
{
    auto tokens = scan("true");

    EXPECT_EQ(tokens[0].getType(), TokenType::TRUE);
    EXPECT_EQ(std::get<bool>(tokens[0].getLiteral()), true);
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTestFixture, IdentifierStartsWithKeyword)
{
    auto tokens = scan("variable");

    EXPECT_EQ(tokens[0].getType(), TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[0].getLexeme(), "variable");
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTestFixture, VarDeclaration)
{
    auto tokens = scan("var a");

    EXPECT_EQ(tokens[0].getType(), TokenType::VAR);
    EXPECT_EQ(tokens[1].getType(), TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[1].getLexeme(), "a");
    EXPECT_EQ(tokens[2].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTestFixture, KeywordFalse)
{
    auto tokens = scan("false");

    EXPECT_EQ(tokens[0].getType(), TokenType::FALSE);
    EXPECT_EQ(std::get<bool>(tokens[0].getLiteral()), false);
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}
