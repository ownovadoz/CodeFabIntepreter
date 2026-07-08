#include "CodeFabFacade.h"

#ifdef _DEBUG

CodeFabFacade::CodeFabFacade()
    : owned_assembler_unit(std::make_unique<AssemblerUnit>()),
    owned_checker(std::make_unique<NoOpChecker>()),
    owned_executor(std::make_unique<Executor>()),
    assembler_unit(owned_assembler_unit.get()),
    checker(owned_checker.get()),
    executor(owned_executor.get()) {
}

CodeFabFacade::CodeFabFacade(IAssemblerUnit& assembler_unit, IChecker& checker, IExecutor& executor)
    : assembler_unit(&assembler_unit), checker(&checker), executor(&executor) {
}

void CodeFabFacade::execute(const string& code_line) {
    Statement* statement = assembler_unit->assemble(code_line);
    checker->run();
    executor->run();
}

#else

void CodeFabFacade::execute(const string& code_line) {
    Statement* statement = assembler_unit.assemble(code_line);
    checker.run();
    executor.run();
}

#endif
