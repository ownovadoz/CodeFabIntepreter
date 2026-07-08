#pragma once
#include "../InterfaceForCodeFabTest.h"

#include <memory>
#include <string>

using std::string;
using std::unique_ptr;

#ifdef _DEBUG
class AssemblerUnit : public IAssemblerUnit {
public:
	unique_ptr<Statement> assemble(const string& code_line) override;
};
#else
class AssemblerUnit {
public:
	unique_ptr<Statement> assemble(const string& code_line);
};
#endif
