#include "Lexer.h"

#include <gmock/gmock.h>
#include <vector>

using std::vector;
using namespace testing;

TEST(LexerTest, SingleCharTokens)
{
    Lexer lexer("( ) { } ; + - * = ! > <");
    vector<Token> tokens = lexer.scanTokens();

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

TEST(LexerTest, SlashToken)
{
    Lexer lexer("/");
    vector<Token> tokens = lexer.scanTokens();

    EXPECT_EQ(tokens[0].getType(), TokenType::SLASH);
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}

TEST(LexerTest, CommentIsIgnored)
{
    Lexer lexer("// this is a comment\n+");
    vector<Token> tokens = lexer.scanTokens();

    EXPECT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].getType(), TokenType::PLUS);
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}

TEST(LexerTest, WhitespaceIsIgnoredAndNewlineIncrementsLine)
{
    Lexer lexer("  +  \t\n-  ");
    vector<Token> tokens = lexer.scanTokens();

    EXPECT_EQ(tokens.size(), 3u);
    EXPECT_EQ(tokens[0].getType(), TokenType::PLUS);
    EXPECT_EQ(tokens[0].getLine(), 1);
    EXPECT_EQ(tokens[1].getType(), TokenType::MINUS);
    EXPECT_EQ(tokens[1].getLine(), 2);
}

TEST(LexerTest, EmptySourceReturnsOnlyEOF)
{
    Lexer lexer("");
    vector<Token> tokens = lexer.scanTokens();

    EXPECT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].getType(), TokenType::END_OF_FILE);
}

TEST(LexerTest, TokenMetaIsCorrect)
{
    Lexer lexer("+");
    vector<Token> tokens = lexer.scanTokens();

    EXPECT_EQ(tokens[0].getLexeme(), "+");
    EXPECT_EQ(tokens[0].getLine(), 1);
}

TEST(LexerTest, MultiCharTokens)
{
    Lexer lexer("== != <= >=");
    vector<Token> tokens = lexer.scanTokens();

    EXPECT_EQ(tokens[0].getType(), TokenType::EQUAL_EQUAL);
    EXPECT_EQ(tokens[1].getType(), TokenType::BANG_EQUAL);
    EXPECT_EQ(tokens[2].getType(), TokenType::LESS_EQUAL);
    EXPECT_EQ(tokens[3].getType(), TokenType::GREATER_EQUAL);
    EXPECT_EQ(tokens[4].getType(), TokenType::END_OF_FILE);
}

TEST(LexerTest, MultiCharTokenLexemeIsCorrect)
{
    Lexer lexer("==");
    vector<Token> tokens = lexer.scanTokens();

    EXPECT_EQ(tokens[0].getLexeme(), "==");
}

TEST(LexerTest, SingleCharNotConsumedByMultiChar)
{
    Lexer lexer("= ! < >");
    vector<Token> tokens = lexer.scanTokens();

    EXPECT_EQ(tokens[0].getType(), TokenType::EQUAL);
    EXPECT_EQ(tokens[1].getType(), TokenType::BANG);
    EXPECT_EQ(tokens[2].getType(), TokenType::LESS);
    EXPECT_EQ(tokens[3].getType(), TokenType::GREATER);
    EXPECT_EQ(tokens[4].getType(), TokenType::END_OF_FILE);
}

TEST(LexerTest, StringLiteral)
{
    Lexer lexer("\"hello\"");
    vector<Token> tokens = lexer.scanTokens();

    EXPECT_EQ(tokens[0].getType(), TokenType::STRING);
    EXPECT_EQ(std::get<string>(tokens[0].getLiteral()), "hello");
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}

TEST(LexerTest, StringLiteralLexemeIncludesQuotes)
{
    Lexer lexer("\"hi\"");
    vector<Token> tokens = lexer.scanTokens();

    EXPECT_EQ(tokens[0].getLexeme(), "\"hi\"");
}

TEST(LexerTest, IntegerLiteral)
{
    Lexer lexer("123");
    vector<Token> tokens = lexer.scanTokens();

    EXPECT_EQ(tokens[0].getType(), TokenType::NUMBER);
    EXPECT_DOUBLE_EQ(std::get<double>(tokens[0].getLiteral()), 123.0);
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}

TEST(LexerTest, FloatLiteral)
{
    Lexer lexer("3.14");
    vector<Token> tokens = lexer.scanTokens();

    EXPECT_EQ(tokens[0].getType(), TokenType::NUMBER);
    EXPECT_DOUBLE_EQ(std::get<double>(tokens[0].getLiteral()), 3.14);
}

TEST(LexerTest, MultipleLiterals)
{
    Lexer lexer("123 \"abc\" 4.5");
    vector<Token> tokens = lexer.scanTokens();

    EXPECT_EQ(tokens[0].getType(), TokenType::NUMBER);
    EXPECT_EQ(tokens[1].getType(), TokenType::STRING);
    EXPECT_EQ(tokens[2].getType(), TokenType::NUMBER);
    EXPECT_EQ(tokens[3].getType(), TokenType::END_OF_FILE);
}
