#include "Lexer.h"

#include <gmock/gmock.h>
#include <vector>

using std::vector;
using namespace testing;

class LexerTest : public ::testing::Test
{
protected:
    vector<Token> scan(const string& source)
    {
        Lexer lexer(source);
        return lexer.scanTokens();
    }
};

TEST_F(LexerTest, SingleCharTokens)
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

TEST_F(LexerTest, SlashToken)
{
    auto tokens = scan("/");

    EXPECT_EQ(tokens[0].getType(), TokenType::SLASH);
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTest, CommentIsIgnored)
{
    auto tokens = scan("// this is a comment\n+");

    EXPECT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].getType(), TokenType::PLUS);
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTest, WhitespaceIsIgnoredAndNewlineIncrementsLine)
{
    auto tokens = scan("  +  \t\n-  ");

    EXPECT_EQ(tokens.size(), 3u);
    EXPECT_EQ(tokens[0].getType(), TokenType::PLUS);
    EXPECT_EQ(tokens[0].getLine(), 1);
    EXPECT_EQ(tokens[1].getType(), TokenType::MINUS);
    EXPECT_EQ(tokens[1].getLine(), 2);
}

TEST_F(LexerTest, EmptySourceReturnsOnlyEOF)
{
    auto tokens = scan("");

    EXPECT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTest, TokenMetaIsCorrect)
{
    auto tokens = scan("+");

    EXPECT_EQ(tokens[0].getLexeme(), "+");
    EXPECT_EQ(tokens[0].getLine(), 1);
}

TEST_F(LexerTest, MultiCharTokens)
{
    auto tokens = scan("== != <= >=");

    EXPECT_EQ(tokens[0].getType(), TokenType::EQUAL_EQUAL);
    EXPECT_EQ(tokens[1].getType(), TokenType::BANG_EQUAL);
    EXPECT_EQ(tokens[2].getType(), TokenType::LESS_EQUAL);
    EXPECT_EQ(tokens[3].getType(), TokenType::GREATER_EQUAL);
    EXPECT_EQ(tokens[4].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTest, MultiCharTokenLexemeIsCorrect)
{
    auto tokens = scan("==");

    EXPECT_EQ(tokens[0].getLexeme(), "==");
}

TEST_F(LexerTest, SingleCharNotConsumedByMultiChar)
{
    auto tokens = scan("= ! < >");

    EXPECT_EQ(tokens[0].getType(), TokenType::EQUAL);
    EXPECT_EQ(tokens[1].getType(), TokenType::BANG);
    EXPECT_EQ(tokens[2].getType(), TokenType::LESS);
    EXPECT_EQ(tokens[3].getType(), TokenType::GREATER);
    EXPECT_EQ(tokens[4].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTest, StringLiteral)
{
    auto tokens = scan("\"hello\"");

    EXPECT_EQ(tokens[0].getType(), TokenType::STRING);
    EXPECT_EQ(std::get<string>(tokens[0].getLiteral()), "hello");
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTest, StringLiteralLexemeIncludesQuotes)
{
    auto tokens = scan("\"hi\"");

    EXPECT_EQ(tokens[0].getLexeme(), "\"hi\"");
}

TEST_F(LexerTest, IntegerLiteral)
{
    auto tokens = scan("123");

    EXPECT_EQ(tokens[0].getType(), TokenType::NUMBER);
    EXPECT_DOUBLE_EQ(std::get<double>(tokens[0].getLiteral()), 123.0);
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTest, FloatLiteral)
{
    auto tokens = scan("3.14");

    EXPECT_EQ(tokens[0].getType(), TokenType::NUMBER);
    EXPECT_DOUBLE_EQ(std::get<double>(tokens[0].getLiteral()), 3.14);
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTest, MultipleLiterals)
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

TEST_F(LexerTest, IdentifierToken)
{
    auto tokens = scan("abc");

    EXPECT_EQ(tokens[0].getType(), TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[0].getLexeme(), "abc");
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTest, KeywordVar)
{
    auto tokens = scan("var");

    EXPECT_EQ(tokens[0].getType(), TokenType::VAR);
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTest, KeywordIf)
{
    auto tokens = scan("if");

    EXPECT_EQ(tokens[0].getType(), TokenType::IF);
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTest, KeywordTrue)
{
    auto tokens = scan("true");

    EXPECT_EQ(tokens[0].getType(), TokenType::TRUE);
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTest, IdentifierStartsWithKeyword)
{
    auto tokens = scan("variable");

    EXPECT_EQ(tokens[0].getType(), TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[0].getLexeme(), "variable");
    EXPECT_EQ(tokens[1].getType(), TokenType::END_OF_FILE);
}

TEST_F(LexerTest, VarDeclaration)
{
    auto tokens = scan("var a");

    EXPECT_EQ(tokens[0].getType(), TokenType::VAR);
    EXPECT_EQ(tokens[1].getType(), TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[1].getLexeme(), "a");
    EXPECT_EQ(tokens[2].getType(), TokenType::END_OF_FILE);
}
