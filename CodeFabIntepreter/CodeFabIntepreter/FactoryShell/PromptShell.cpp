#include "PromptShell.h"
#include "../CodeFabException.h"

#include <iostream>
#include <string>

using std::cout;
using std::string;

void PromptShell::enter() {
    cout << "CodeFab Interpreter - exit / quit로 종료\n";
    cout << "> ";

    string input_line;
    string accumulated;
    int brace_depth = 0;
    bool in_string = false;

    while (std::getline(std::cin, input_line)) {
        if (input_line == "exit" || input_line == "quit")
            break;

        for (char c : input_line) {
            if (c == '"') in_string = !in_string;
            if (!in_string) {
                if (c == '{') brace_depth++;
                else if (c == '}') brace_depth--;
            }
        }

        accumulated = accumulated.empty() ? input_line : accumulated + "\n" + input_line;

        if (brace_depth <= 0) {
            brace_depth = 0;
            in_string = false;
            code_line = accumulated;
            try {
                code_fab_facade.execute(accumulated);
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
            accumulated.clear();
            cout << "> ";
        }
        else {
            cout << "  ";
        }
    }

    cout << "CodeFab Interpreter Exit\n";
}