#include "CodeFabNamespace.h"

#include "../CodeFabException.h"

#include <stdexcept>

Value CodeFabNamespace::get(const Token& name) const
{
    auto found = members.find(name.getLexeme());
    if (found != members.end()) return found->second;

    throw CodeFabException(name, "존재하지 않는 이름입니다: '" + name.getLexeme() + "'");
}

void CodeFabNamespace::define(const string& name, const Value& value)
{
    members[name] = value;
}

int CodeFabNamespace::arity() const
{
    return 0;
}

Value CodeFabNamespace::call(Interpreter&, const vector<Value>&) const
{
    // Interpreter::evaluateCallExpr이 네임스페이스 호출을 미리 걸러내므로 실제로는 호출되지 않는다.
    throw std::logic_error("CodeFabNamespace는 호출할 수 없습니다.");
}

string CodeFabNamespace::toString() const
{
    return "namespace";
}
