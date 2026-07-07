#pragma once
#include <string>

using std::string;

class PromptShell {
public:
	void runPrompt();
	const string& getLine() const { return line; }
private:
	string line;
};