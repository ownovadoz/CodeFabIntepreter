#pragma once
#include "InterfaceForCodeFabTest.h"
#include "AssemblerUnit/AssemblerUnit.h"
#include "AssemblerUnit/Parser/Statement.h"
#include "CheckerUnit/Checker.h"
#include "ExecutorUnit/Interpreter.h"
#include <memory>
#include <string>
#include <vector>

using std::string;
using std::unique_ptr;
using std::vector;

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

    // Func 선언은 Executor가 선언 시점의 FunctionStmt를 가리키는 포인터를
    // Environment에 값으로 저장해 두므로, 그 AST는 CodeFabFacade가 살아있는
    // 동안(REPL의 여러 줄에 걸쳐서도) 계속 유효해야 한다. 각 줄에서 조립된
    // 문장들을 여기 보관해 execute() 호출이 끝나도 소멸되지 않게 한다.
    vector<vector<unique_ptr<Statement>>> retained_statements;
};

