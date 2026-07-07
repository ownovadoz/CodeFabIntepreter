#include "VariableScopeChecker.h"

#include <algorithm>

CheckResult CheckResult::isOK()
{
    return CheckResult{ false, "" };
}

CheckResult CheckResult::checkerUnitHasError(std::string message)
{
    return CheckResult{ true, std::move(message) };
}

void VariableScopeChecker::enterScope()
{
    scope_stack.emplace_back();
}

void VariableScopeChecker::exitScope()
{
    if (scope_stack.empty()) return;

    scope_stack.pop_back();
}

CheckResult VariableScopeChecker::declareVariable(const std::string& name, const std::vector<std::string>& initializer_references)
{
    if (isDeclaredInCurrentScope(name))
        return CheckResult::checkerUnitHasError("이미 해당 변수는 현재 스코프에서 사용중입니다: '" + name + "'");

    bool is_self_referenced = std::find(initializer_references.begin(), initializer_references.end(), name) != initializer_references.end();
    if (is_self_referenced)
        return CheckResult::checkerUnitHasError("자신의 초기화식에서 지역변수를 읽을 수 없습니다: '" + name + "'");

    scope_stack.back().insert(name);
    return CheckResult::isOK();
}

bool VariableScopeChecker::isDeclaredInCurrentScope(const std::string& name) const
{
    if (scope_stack.empty()) return false;

    return scope_stack.back().count(name) > 0;
}
