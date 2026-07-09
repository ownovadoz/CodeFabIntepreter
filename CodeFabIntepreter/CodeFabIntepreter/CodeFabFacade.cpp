#include "CodeFabFacade.h"
#include "AssemblerUnit/Parser/Statement.h"

#include <memory>
#include <vector>

using std::unique_ptr;
using std::vector;

namespace {
    // 조립된 문장 전체를 먼저 검사한 뒤에야 실행에 들어간다. 문장 하나를
    // 검사/실행할 때마다 번갈아 처리하면, 뒤쪽 문장의 오류를 미처 걸러내지
    // 못한 채 앞쪽 문장이 이미 실행되어 버리는 상황이 생길 수 있다.
    template <typename CheckerT, typename ExecutorT>
    void runPipeline(vector<unique_ptr<Statement>>& statements, CheckerT& checker, ExecutorT& executor) {
        checker.check(statements);
        executor.interpret(statements);
    }
}

#ifdef _DEBUG

CodeFabFacade::CodeFabFacade()
    : owned_assembler_unit(std::make_unique<AssemblerUnit>()),
    owned_checker(std::make_unique<Checker>()),
    owned_executor(std::make_unique<Interpreter>()),
    assembler_unit(owned_assembler_unit.get()),
    checker(owned_checker.get()),
    executor(owned_executor.get()) {
}

CodeFabFacade::CodeFabFacade(IAssemblerUnit& assembler_unit, IChecker& checker, IExecutor& executor)
    : assembler_unit(&assembler_unit), checker(&checker), executor(&executor) {
}

void CodeFabFacade::execute(const string& code_line) {
    retained_statements.push_back(assembler_unit->assemble(code_line));
    runPipeline(retained_statements.back(), *checker, *executor);
}

#else

void CodeFabFacade::execute(const string& code_line) {
    retained_statements.push_back(assembler_unit.assemble(code_line));
    runPipeline(retained_statements.back(), checker, executor);
}

#endif
