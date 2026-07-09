#pragma once
#include "IShellMode.h"
#include "../CodeFabFacade.h"

#include <string>

using std::string;

class PromptShell : public IShellMode {
public:
	void enter() override;
	const string& getLine() const { return code_line; }
private:
	string code_line;
	CodeFabFacade code_fab_facade;
};
