#pragma once
#include <string>

using std::string;

#ifdef _DEBUG
class ITokenizer {
public:
	virtual ~ITokenizer() = default;
	virtual void run(const string& code_line) = 0;
};

class IChecker {
public:
	virtual ~IChecker() = default;
	virtual void run() = 0;
};

class IExecutor {
public:
	virtual ~IExecutor() = default;
	virtual void run() = 0;
};

class Tokenizer : public ITokenizer {
public:
	void run(const string& code_line) override {

	}
};

class Checker : public IChecker {
public:
	void run() override {

	}
};

class Executor : public IExecutor {
public:
	void run() override {

	}
};
#else
class Tokenizer {
public:
	void run(const string& code_line) {

	}
};

class Checker {
public:
	void run() {

	}
};

class Executor {
public:
	void run() {

	}
};
#endif