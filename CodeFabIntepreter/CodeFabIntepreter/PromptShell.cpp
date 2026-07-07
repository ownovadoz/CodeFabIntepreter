#include "PromptShell.h"
#include <iostream>
#include <string>
using std::cout;
using std::string;

void PromptShell::runPrompt() {
    std::cout << "CodeFab Interpreter - Ctrl+D / Ctrl+Z / exit / EXIT 로 종료\n";
    std::cout << "> ";

    while (std::getline(std::cin, line))
    {
        std::cout << "> ";
    }
    std::cout << "\n";
    return;
}