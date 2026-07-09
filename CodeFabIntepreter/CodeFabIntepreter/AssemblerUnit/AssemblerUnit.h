#pragma once
#include "../InterfaceForCodeFabTest.h"

#include <memory>
#include <string>
#include <vector>

using std::string;
using std::unique_ptr;
using std::vector;

#ifdef _DEBUG
class AssemblerUnit : public IAssemblerUnit {
public:
	vector<unique_ptr<Statement>> assemble(const string& code_line) override;
};
#else
class AssemblerUnit {
public:
	vector<unique_ptr<Statement>> assemble(const string& code_line);
};
#endif
