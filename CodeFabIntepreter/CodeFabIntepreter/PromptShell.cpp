#include <iostream>
#include <string>

#include "PromptShell.h"

using std::cout;
using std::string;

void PromptShell::runPrompt() {
    cout << "CodeFab Interpreter - Ctrl+D / Ctrl+Z / exit / EXIT 후 enter로 종료\n";
    cout << "> ";

    bool first_line_handled = false;
    string code_line;
    while (std::getline(std::cin, code_line)) {
        if (code_line == "exit" || code_line == "EXIT")
            break;
        if (first_line_handled)
            break;

        line = code_line;
        first_line_handled = true;
		code_fab_facade.execute(code_line);
        cout << "> ";
    }
    cout << "CodeFab Interpreter Exit\n";
    return;
}