#include "CodeFabFunction.h"

#include "Interpreter.h"
#include "ReturnSignal.h"

#include <memory>
#include <utility>

using std::make_shared;
using std::move;

CodeFabFunction::CodeFabFunction(const FunctionStmt* declaration, shared_ptr<Environment> closure)
    : declaration{ declaration }, closure{ move(closure) }
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

    try {
        interpreter.executeBlockWithEnvironment(declaration->getBody(), call_environment);
    }
    catch (const ReturnSignal& signal) {
        return signal.getValue();
    }

    return Value();
}

string CodeFabFunction::toString() const
{
    return "<fn " + declaration->getName().getLexeme() + ">";
}
