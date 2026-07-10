#include "PromptShell.h"
#include "shell_exception_reporter.h"

#include <iostream>
#include <string>

using std::cout;
using std::string;

void PromptShell::enter() {
    cout << "CodeFab Interpreter - exit / quit로 종료\n";
    cout << "> ";

    string input_line;
    while (std::getline(std::cin, input_line)) {
        if (input_line == "exit" || input_line == "quit")
            break;
        size_t escaped_newline_pos = input_line.find("\\n");
        if (escaped_newline_pos != string::npos) {
            code_line = input_line.substr(0, escaped_newline_pos);
        }
        else {
            code_line = input_line;
        }
        reportShellExceptions([&] { code_fab_facade.execute(code_line); });
        cout << "> ";
    }

    cout << "CodeFab Interpreter Exit\n";
    return;
}