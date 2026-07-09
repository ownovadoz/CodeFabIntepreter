#include "PromptShell.h"
#include "../CodeFabException.h"

#include <iostream>
#include <sstream>
#include <string>

using std::cout;
using std::istringstream;
using std::string;

void PromptShell::enter() {
    cout << "CodeFab Interpreter - exit / quit로 종료\n";
    cout << "> ";

    string raw_line;
    string accumulated;
    int brace_depth = 0;
    bool in_string = false;

    while (std::getline(std::cin, raw_line)) {
        if (raw_line == "exit" || raw_line == "EXIT" || raw_line == "quit")
            break;

        // 붙여넣기 시 \\n(두 글자)으로 들어오는 개행을 실제 개행으로 변환
        string::size_type pos = 0;
        while ((pos = raw_line.find("\\n", pos)) != string::npos) {
            raw_line.replace(pos, 2, "\n");
            pos += 1;
        }

        istringstream sub_stream(raw_line);
        string input_line;
        while (std::getline(sub_stream, input_line)) {
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
    }

    cout << "CodeFab Interpreter Exit\n";
}