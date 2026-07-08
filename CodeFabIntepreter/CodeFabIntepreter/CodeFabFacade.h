#pragma once
#include "InterfaceForCodeFabTest.h"
#include "AssemblerUnit/AssemblerUnit.h"
#include "CheckerUnit/Checker.h"
#include "ExecutorUnit/Interpreter.h"
#include <string>

#ifdef _DEBUG
#include <memory>
#endif

using std::string;

class CodeFabFacade {
public:
#ifdef _DEBUG
    CodeFabFacade();
    CodeFabFacade(IAssemblerUnit& assembler_unit, IChecker& checker, IExecutor& executor);
#endif

    void execute(const string& code_line);

private:
#ifdef _DEBUG
    std::unique_ptr<IAssemblerUnit> owned_assembler_unit;
    std::unique_ptr<IChecker> owned_checker;
    std::unique_ptr<IExecutor> owned_executor;

    IAssemblerUnit* assembler_unit;
    IChecker* checker;
    IExecutor* executor;
#else
    AssemblerUnit assembler_unit;
    Checker checker;
    Interpreter executor;
#endif
};

