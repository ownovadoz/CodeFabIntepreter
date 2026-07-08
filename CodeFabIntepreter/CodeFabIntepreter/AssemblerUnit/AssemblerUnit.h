#pragma once
#include "../InterfaceForCodeFabTest.h"

#include <string>

using std::string;

#ifdef _DEBUG
class AssemblerUnit : public IAssemblerUnit {
public:
	Statement* assemble(const string& code_line) override;
};
#else
class AssemblerUnit {
public:
	Statement* assemble(const string& code_line);
};
#endif
