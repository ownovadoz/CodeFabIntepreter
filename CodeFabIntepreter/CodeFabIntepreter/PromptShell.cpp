#include <iostream>
#include <string>

#include "PromptShell.h"

using std::cout;
using std::string;

void PromptShell::runPrompt() {
    cout << "CodeFab Interpreter - Ctrl+D / Ctrl+Z / exit / EXIT 후 enter로 종료\n";
    cout << "> ";

    bool firstLineHandled = false;
    string codeLine;
    while (std::getline(std::cin, codeLine)){
        if (codeLine == "exit" || codeLine == "EXIT")
            break;
        if (firstLineHandled)
            break;

        line = codeLine;
        firstLineHandled = true;
        cout << "> ";
    }
    cout << "CodeFab Interpreter Exit\n";
    return;
}