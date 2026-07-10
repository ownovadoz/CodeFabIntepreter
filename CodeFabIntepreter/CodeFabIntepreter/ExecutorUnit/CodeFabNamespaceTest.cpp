#include "CodeFabNamespace.h"

#include "../AssemblerUnit/Tokenizer/Token.h"
#include "../CodeFabException.h"

#include <gmock/gmock.h>

#include <variant>

using std::get;
using std::monostate;
using std::string;

namespace {
    Token identifier(const string& name) {
        return Token(TokenType::IDENTIFIER, name, monostate{}, 1);
    }
}

class CodeFabNamespaceTestFixture : public testing::Test {
public:
    CodeFabNamespace module_namespace;
};

TEST_F(CodeFabNamespaceTestFixture, DefineThenGetReturnsTheStoredValue) {
    module_namespace.define("speed", 10.0);

    EXPECT_EQ(get<double>(module_namespace.get(identifier("speed"))), 10.0);
}

TEST_F(CodeFabNamespaceTestFixture, GetOnUndefinedNameThrowsCodeFabException) {
    EXPECT_THROW(module_namespace.get(identifier("missing")), CodeFabException);
}

TEST_F(CodeFabNamespaceTestFixture, ArityIsZero) {
    EXPECT_EQ(module_namespace.arity(), 0);
}

TEST_F(CodeFabNamespaceTestFixture, ToStringReportsNamespace) {
    EXPECT_EQ(module_namespace.toString(), "namespace");
}
