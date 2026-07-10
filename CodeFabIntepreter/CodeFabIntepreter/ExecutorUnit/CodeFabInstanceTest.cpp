#include "CodeFabInstance.h"

#include "CodeFabClass.h"
#include "CodeFabFunction.h"
#include "Environment.h"

#include "../AssemblerUnit/Parser/Statement.h"
#include "../AssemblerUnit/Tokenizer/Token.h"
#include "../CodeFabException.h"

#include <gmock/gmock.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

using std::get;
using std::make_shared;
using std::make_unique;
using std::monostate;
using std::shared_ptr;
using std::string;
using std::unordered_map;

namespace {
    Token identifier(const string& name) {
        return Token(TokenType::IDENTIFIER, name, monostate{}, 1);
    }

    shared_ptr<CodeFabClass> emptyClass(const string& name) {
        return make_shared<CodeFabClass>(name, nullptr, unordered_map<string, shared_ptr<CodeFabFunction>>{});
    }
}

TEST(CodeFabInstanceTestFixture, SetThenGetReturnsTheStoredFieldValue) {
    CodeFabInstance instance(emptyClass("Robot"));

    instance.set(identifier("speed"), 10.0);

    EXPECT_EQ(get<double>(instance.get(identifier("speed"))), 10.0);
}

TEST(CodeFabInstanceTestFixture, SetOverwritesAnExistingField) {
    CodeFabInstance instance(emptyClass("Robot"));

    instance.set(identifier("speed"), 10.0);
    instance.set(identifier("speed"), 20.0);

    EXPECT_EQ(get<double>(instance.get(identifier("speed"))), 20.0);
}

TEST(CodeFabInstanceTestFixture, GetOnUndefinedFieldThrowsCodeFabException) {
    CodeFabInstance instance(emptyClass("Robot"));

    EXPECT_THROW(instance.get(identifier("power")), CodeFabException);
}

TEST(CodeFabInstanceTestFixture, GetFallsBackToBoundMethodWhenNoFieldMatches) {
    FunctionStmt move_decl(identifier("move"), {}, make_unique<BlockStmt>());
    auto move_fn = make_shared<CodeFabFunction>(&move_decl, make_shared<Environment>());
    auto klass = make_shared<CodeFabClass>("Robot", nullptr, unordered_map<string, shared_ptr<CodeFabFunction>>{ { "move", move_fn } });

    auto instance = make_shared<CodeFabInstance>(klass);
    Value method_value = instance->get(identifier("move"));

    auto* callable = std::get_if<shared_ptr<Callable>>(&method_value);
    ASSERT_NE(callable, nullptr);
    EXPECT_NE(std::dynamic_pointer_cast<CodeFabFunction>(*callable), nullptr);
}

TEST(CodeFabInstanceTestFixture, ToStringIncludesClassName) {
    CodeFabInstance instance(emptyClass("Robot"));

    EXPECT_EQ(instance.toString(), "Robot instance");
}

TEST(CodeFabInstanceTestFixture, ArityIsZeroAndCallThrows) {
    CodeFabInstance instance(emptyClass("Robot"));

    EXPECT_EQ(instance.arity(), 0);
}
