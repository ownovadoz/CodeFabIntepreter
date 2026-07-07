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
}

void VariableScopeChecker::exitScope()
{
}

CheckResult VariableScopeChecker::declareVariable(const std::string& name, const std::vector<std::string>& initializer_references)
{
    return CheckResult::isOK();
}

bool VariableScopeChecker::isDeclaredInCurrentScope(const std::string& name) const
{
    return 0;
}
