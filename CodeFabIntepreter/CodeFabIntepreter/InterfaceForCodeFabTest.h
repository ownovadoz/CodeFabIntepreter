#pragma once
#include <memory>
#include <string>
#include <vector>

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
	virtual void check(Statement* root) = 0;
};

class IExecutor {
public:
	virtual ~IExecutor() = default;
	virtual void interpret(Statement* root) = 0;
};
#endif