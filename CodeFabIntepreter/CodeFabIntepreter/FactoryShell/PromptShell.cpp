#include "PromptShell.h"
#include "../CodeFabException.h"

#include <iostream>
#include <string>

using std::cout;
using std::string;

void PromptShell::enter() {
    cout << "CodeFab Interpreter - Ctrl+D / Ctrl+Z / exit / EXIT 후 enter로 종료\n";
    cout << "> ";

    string input_line;
    while (std::getline(std::cin, input_line)) {
        if (input_line == "exit" || input_line == "EXIT")
            break;
        size_t escaped_newline_pos = input_line.find("\\n");
        if (escaped_newline_pos != string::npos) {
            code_line = input_line.substr(0, escaped_newline_pos);
        }
        else {
            code_line = input_line;
        }
        try {
            code_fab_facade.execute(code_line);
        }
        catch (const CodeFabException& exception) {
            std::cerr << exception.what() << std::endl;
        }
        catch (const std::exception& exception) {
            std::cerr << "[unexpected error] " << exception.what() << std::endl;
        }
        catch (...) {
            std::cerr << "[unexpected error] unknown exception" << std::endl;
        }
        cout << "> ";
    }

    cout << "CodeFab Interpreter Exit\n";
    return;
}