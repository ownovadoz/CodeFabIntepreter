#include "CodeFabFacade.h"
#include "AssemblerUnit/Parser/Statement.h"
#include "CodeFabException.h"

#include <exception>
#include <iostream>

namespace {
    void reportException(const CodeFabException& exception) {
        std::cerr << exception.what() << std::endl;
    }

    void reportException(const std::exception& exception) {
        std::cerr << "[unexpected error] " << exception.what() << std::endl;
    }

    void reportUnknownException() {
        std::cerr << "[unexpected error] unknown exception" << std::endl;
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
    try {
        unique_ptr<Statement> statement = assembler_unit->assemble(code_line);
        checker->check(statement.get());
        executor->interpret(statement.get());
    }
    catch (const CodeFabException& exception) {
        reportException(exception);
    }
    catch (const std::exception& exception) {
        reportException(exception);
    }
    catch (...) {
        reportUnknownException();
    }
}

#else

void CodeFabFacade::execute(const string& code_line) {
    try {
        unique_ptr<Statement> statement = assembler_unit.assemble(code_line);
        checker.check(statement.get());
        executor.interpret(statement.get());
    }
    catch (const CodeFabException& exception) {
        reportException(exception);
    }
    catch (const std::exception& exception) {
        reportException(exception);
    }
    catch (...) {
        reportUnknownException();
    }
}

#endif
