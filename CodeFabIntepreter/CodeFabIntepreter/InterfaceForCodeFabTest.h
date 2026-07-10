#pragma once
#include "ExecutorUnit/Environment.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

using std::function;
using std::string;
using std::unique_ptr;
using std::vector;

class Statement;

#ifdef _DEBUG
class IAssemblerUnit {
public:
	virtual ~IAssemblerUnit() = default;
	virtual vector<unique_ptr<Statement>> assemble(const string& code_line) = 0;
};

class IChecker {
public:
	virtual ~IChecker() = default;
	virtual void check(const vector<unique_ptr<Statement>>& statements) = 0;
};

class IExecutor {
public:
	virtual ~IExecutor() = default;
	virtual void interpret(const vector<unique_ptr<Statement>>& statements) = 0;
	virtual void setBeforeStatementHook(function<void(int line)> hook) = 0;
	virtual vector<VariableSnapshot> inspectVariables() const = 0;
};
#endif