#include "CodeFabNamespace.h"
#include "Interpreter.h"

#include "../AssemblerUnit/Tokenizer/Token.h"
#include "../CodeFabException.h"

#include <gmock/gmock.h>

#include <stdexcept>
#include <variant>
#include <vector>

using std::get;
using std::monostate;
using std::string;
using std::vector;

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

TEST_F(CodeFabNamespaceTestFixture, CallThrowsLogicError) {
    // Interpreter::evaluateCallExpr이 네임스페이스 호출을 미리 걸러내므로 실제로는
    // 도달하지 않는 방어적 구현이지만, 그 구현 자체는 직접 검증해둔다.
    Interpreter interpreter;

    EXPECT_THROW(module_namespace.call(interpreter, vector<Value>{}), std::logic_error);
}
