#include "PromptShell.h"
#include <iostream>
#include <string>
using std::cout;
using std::string;

void PromptShell::runPrompt() {
    std::cout << "CodeFab Interpreter - Ctrl+D / Ctrl+Z / exit / EXIT 후 enter로 종료\n";
    std::cout << "> ";

    bool captured = false;
    std::string currentLine;
    while (std::getline(std::cin, currentLine))
    {
        if (!captured)
        {
            line = currentLine;
            captured = true;
        }
        std::cout << "> ";
    }
    std::cout << "\n";
    return;
}