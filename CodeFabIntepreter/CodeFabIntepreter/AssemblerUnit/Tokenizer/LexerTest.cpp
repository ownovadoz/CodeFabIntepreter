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
