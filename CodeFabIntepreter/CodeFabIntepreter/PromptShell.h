#pragma once
#include "CodeFabFacade.h"

#include <string>

using std::string;

class PromptShell {
public:
	void runPrompt();
	const string& getLine() const { return code_line; }
private:
	string code_line;
	CodeFabFacade code_fab_facade;
};