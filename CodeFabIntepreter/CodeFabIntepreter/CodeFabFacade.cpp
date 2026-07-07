#include "CodeFabFacade.h"

#ifdef _DEBUG

CodeFabFacade::CodeFabFacade()
    : owned_tokenizer(std::make_unique<Tokenizer>()),
    owned_checker(std::make_unique<Checker>()),
    owned_executor(std::make_unique<Executor>()),
    tokenizer(owned_tokenizer.get()),
    checker(owned_checker.get()),
    executor(owned_executor.get()) {
}

CodeFabFacade::CodeFabFacade(ITokenizer& tokenizer, IChecker& checker, IExecutor& executor)
    : tokenizer(&tokenizer), checker(&checker), executor(&executor) {
}

void CodeFabFacade::execute(const string& code_line) {
    tokenizer->run(code_line);
    checker->run();
    executor->run();
}

#else

void CodeFabFacade::execute(const string& code_line) {
    tokenizer.run(code_line);
    checker.run();
    executor.run();
}

#endif
