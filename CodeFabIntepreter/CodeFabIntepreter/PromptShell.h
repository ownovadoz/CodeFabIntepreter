#pragma once
#include <string>

class PromptShell {
public:
	void runPrompt();
	const std::string& getLine() const { return line; }
private:
	std::string line;
};