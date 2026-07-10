#pragma once
#include "InterfaceForCodeFabTest.h"
#include "AssemblerUnit/AssemblerUnit.h"
#include "CheckerUnit/Checker.h"
#include "ExecutorUnit/Interpreter.h"
#include <functional>
#include <string>

#ifdef _DEBUG
#include <memory>
#endif

using std::function;
using std::string;

class CodeFabFacade {
public:
#ifdef _DEBUG
    CodeFabFacade();
    CodeFabFacade(IAssemblerUnit& assembler_unit, IChecker& checker, IExecutor& executor);
#endif

    void execute(const string& code_line);

    // Stmt 단위 stepping/breakpoint를 지원하기 위해 Interpreter의
    // before-statement 훅 등록을 그대로 전달한다.
    void setBeforeStatementHook(function<void(int line)> hook);

    // watch/inspect가 정지 시점의 변수 스냅샷을 조회하는 데 사용한다.
    vector<VariableSnapshot> inspectVariables() const;

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

