#include "CodeFabInstance.h"

#include "../CodeFabException.h"

#include <stdexcept>
#include <utility>

CodeFabInstance::CodeFabInstance(shared_ptr<CodeFabClass> klass) : klass{ std::move(klass) }
{
}

Value CodeFabInstance::get(const Token& name)
{
    auto found = fields.find(name.getLexeme());
    if (found != fields.end()) return found->second;

    shared_ptr<CodeFabFunction> method = klass->findMethod(name.getLexeme());
    if (method) return method->bind(shared_from_this());

    throw CodeFabException(name, "존재하지 않는 필드입니다: '" + name.getLexeme() + "'");
}

void CodeFabInstance::set(const Token& name, const Value& value)
{
    fields[name.getLexeme()] = value;
}

const shared_ptr<CodeFabClass>& CodeFabInstance::getClass() const
{
    return klass;
}

int CodeFabInstance::arity() const
{
    return 0;
}

Value CodeFabInstance::call(Interpreter&, const vector<Value>&) const
{
    // Interpreter::evaluateCallExpr이 인스턴스 호출을 미리 걸러내므로 실제로는 호출되지 않는다.
    throw std::logic_error("CodeFabInstance는 호출할 수 없습니다.");
}

string CodeFabInstance::toString() const
{
    return klass->getName() + " instance";
}
