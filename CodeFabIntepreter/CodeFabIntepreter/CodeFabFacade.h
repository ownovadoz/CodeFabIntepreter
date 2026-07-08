#pragma once
#include "InterfaceForCodeFabTest.h"
#include <string>

#ifdef _DEBUG
#include <memory>
#endif

using std::string;

class CodeFabFacade {
public:
#ifdef _DEBUG
    CodeFabFacade();
    CodeFabFacade(ITokenizer& tokenizer, IChecker& checker, IExecutor& executor);
#endif

    void execute(const string& code_line);

private:
#ifdef _DEBUG
    std::unique_ptr<ITokenizer> owned_tokenizer;
    std::unique_ptr<IChecker> owned_checker;
    std::unique_ptr<IExecutor> owned_executor;

    ITokenizer* tokenizer;
    IChecker* checker;
    IExecutor* executor;
#else
    Tokenizer tokenizer;
    NoOpChecker checker;
    Executor executor;
#endif
};

