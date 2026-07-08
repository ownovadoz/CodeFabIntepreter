#pragma once
#include <memory>
#include <string>

using std::string;
using std::unique_ptr;

class Statement;

#ifdef _DEBUG
class IAssemblerUnit {
public:
	virtual ~IAssemblerUnit() = default;
	virtual unique_ptr<Statement> assemble(const string& code_line) = 0;
};

class IChecker {
public:
	virtual ~IChecker() = default;
	virtual void check(Statement* root) = 0;
};

class IExecutor {
public:
	virtual ~IExecutor() = default;
	virtual void run() = 0;
};

class Executor : public IExecutor {
public:
	void run() override {

	}
};
#else
class Executor {
public:
	void run() {

	}
};
#endif