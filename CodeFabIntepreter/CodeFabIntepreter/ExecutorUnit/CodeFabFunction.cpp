#include "CodeFabFunction.h"

#include "CodeFabInstance.h"
#include "Interpreter.h"
#include "ReturnSignal.h"

#include <memory>
#include <utility>

using std::make_shared;
using std::move;

CodeFabFunction::CodeFabFunction(const FunctionStmt* declaration, shared_ptr<Environment> closure, bool is_initializer)
    : declaration{ declaration }, closure{ move(closure) }, is_initializer{ is_initializer }
{
}

int CodeFabFunction::arity() const
{
    return static_cast<int>(declaration->getParams().size());
}

Value CodeFabFunction::call(Interpreter& interpreter, const vector<Value>& arguments) const
{
    shared_ptr<Environment> call_environment = make_shared<Environment>(closure);

    const vector<Token>& params = declaration->getParams();
    for (size_t i = 0; i < params.size(); i++)
        call_environment->define(params[i].getLexeme(), arguments[i]);

    Token this_token(TokenType::IDENTIFIER, "this", Value(), declaration->getName().getLine());

    try {
        interpreter.executeBlockWithEnvironment(declaration->getBody(), call_environment);
    }
    catch (const ReturnSignal& signal) {
        // init()은 Checker가 return 자체를 막아두었으므로, 여기 도달했다면 몸통 밖으로
        // 나가는 예외적인 return일 수 없다. 그래도 항상 인스턴스를 반환하도록 강제한다.
        return is_initializer ? closure->get(this_token) : signal.getValue();
    }

    return is_initializer ? closure->get(this_token) : Value();
}

string CodeFabFunction::toString() const
{
    return "<fn " + declaration->getName().getLexeme() + ">";
}

shared_ptr<CodeFabFunction> CodeFabFunction::bind(shared_ptr<CodeFabInstance> instance) const
{
    shared_ptr<Environment> bound_environment = make_shared<Environment>(closure);
    bound_environment->define("this", move(instance));

    return make_shared<CodeFabFunction>(declaration, bound_environment, is_initializer);
}
