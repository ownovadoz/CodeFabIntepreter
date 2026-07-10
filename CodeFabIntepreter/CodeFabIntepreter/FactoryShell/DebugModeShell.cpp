#include "DebugModeShell.h"

#include <iostream>
#include <sstream>

using std::cin;
using std::cout;
using std::istringstream;

void DebugModeShell::afterLoad(const string& path) {
	cout << "[DEBUG] 소스코드 로딩: " << path << "\n";
}

void DebugModeShell::beforeExecuteLine(int line_number, const string&) {
	// AssemblerUnit은 execute() 호출마다 새로 렉싱하므로, 훅이 넘겨주는 줄 번호는
	// 이번 호출 문자열 안에서의 상대 줄 번호(항상 1)일 뿐이다. 파일 전체 기준
	// 절대 줄 번호는 이 루프가 알고 있으므로, 매 호출마다 그 값으로 다시 바인딩한다.
	code_fab_facade.setBeforeStatementHook([this, line_number](int) { onBeforeStatement(line_number); });
}

void DebugModeShell::onBeforeStatement(int line) {
	bool is_breakpoint = breakpoints.count(line) > 0;
	bool should_pause = mode != Mode::Continue || is_breakpoint;
	if (!should_pause) return;

	printPauseMessage(line, is_breakpoint);
	while (!processCommand(defaultReadCommand())) {}
}

string DebugModeShell::defaultReadCommand() {
	cout << "> ";
	string line;
	if (!std::getline(cin, line)) return "";
	return line;
}

void DebugModeShell::printPauseMessage(int line, bool is_breakpoint_hit) {
	string text = (line >= 1 && static_cast<size_t>(line) <= loaded_lines.size()) ? loaded_lines[line - 1] : "";

	string message = "[DEBUG] " + std::to_string(line) + "번째 줄에서 정지";
	if (is_breakpoint_hit) message += " (breakpoint)";
	message += " → " + text;

	cout << message << "\n";
}

bool DebugModeShell::processCommand(const string& raw_command) {
	istringstream iss(raw_command);
	string command;
	iss >> command;

	if (command.empty()) {
		// 입력 스트림 종료(EOF)를 무한 대기 대신 continue로 취급한다.
		mode = Mode::Continue;
		return true;
	}
	if (command == "step") {
		mode = Mode::Step;
		return true;
	}
	if (command == "next") {
		mode = Mode::Next;
		return true;
	}
	if (command == "continue") {
		mode = Mode::Continue;
		return true;
	}
	if (command == "break") {
		int line_number;
		if (iss >> line_number) {
			breakpoints.insert(line_number);
			cout << "[DEBUG] " << line_number << "번째 줄에 breakpoint 설정\n";
		}
		return false;
	}
	if (command == "remove") {
		int line_number;
		if (iss >> line_number) {
			breakpoints.erase(line_number);
			cout << "[DEBUG] " << line_number << "번째 줄의 breakpoint 해제\n";
		}
		return false;
	}
	if (command == "breakpoints") {
		if (breakpoints.empty()) {
			cout << "[DEBUG] 설정된 breakpoint가 없습니다\n";
		}
		else {
			string joined;
			for (int bp : breakpoints) {
				if (!joined.empty()) joined += ", ";
				joined += std::to_string(bp);
			}
			cout << "[DEBUG] breakpoints: " << joined << "\n";
		}
		return false;
	}

	cout << "[DEBUG] 알 수 없는 명령어: " << raw_command << "\n";
	return false;
}
