#include <iostream>
#include <string>

#include "PromptShell.h"

using std::cout;
using std::string;

void PromptShell::runPrompt() {
    cout << "CodeFab Interpreter - Ctrl+D / Ctrl+Z / exit / EXIT 후 enter로 종료\n";
    cout << "> ";

    string code_line;
    while (std::getline(std::cin, code_line)) {
        if (code_line == "exit" || code_line == "EXIT")
            break;
        size_t escaped_newline_pos = code_line.find("\\n");
        if (escaped_newline_pos != string::npos) {
            code_line = code_line.substr(0, escaped_newline_pos);
        }
        code_fab_facade.execute(code_line);
        cout << "> ";
    }

    cout << "CodeFab Interpreter Exit\n";
    return;
}