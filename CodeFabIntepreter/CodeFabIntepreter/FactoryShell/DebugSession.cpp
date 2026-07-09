#include "DebugSession.h"

#include <iostream>
#include <sstream>
#include <utility>

using std::cin;
using std::cout;
using std::getline;
using std::istringstream;
using std::move;

#ifdef _DEBUG

DebugSession::DebugSession(vector<string> source_lines, function<string()> read_command, function<void(const string&)> write_line)
	: source_lines(move(source_lines)), read_command(move(read_command)), write_line(move(write_line)) {}

void DebugSession::beforeStatement(int line) {
	bool is_breakpoint = breakpoints.count(line) > 0;
	bool should_pause = mode != Mode::Continue || is_breakpoint;
	if (!should_pause) return;

	printPauseMessage(line, mode == Mode::Continue && is_breakpoint, write_line);
	while (!processCommand(read_command(), write_line)) {}
}

#else

DebugSession::DebugSession(vector<string> source_lines) : source_lines(move(source_lines)) {}

void DebugSession::beforeStatement(int line) {
	bool is_breakpoint = breakpoints.count(line) > 0;
	bool should_pause = mode != Mode::Continue || is_breakpoint;
	if (!should_pause) return;

	printPauseMessage(line, mode == Mode::Continue && is_breakpoint, defaultWriteLine);
	while (!processCommand(defaultReadCommand(), defaultWriteLine)) {}
}

#endif

string DebugSession::defaultReadCommand() {
	cout << "> ";
	string line;
	if (!getline(cin, line)) return "";
	return line;
}

void DebugSession::defaultWriteLine(const string& message) {
	cout << message << "\n";
}

void DebugSession::printPauseMessage(int line, bool is_breakpoint_hit, const function<void(const string&)>& writer) {
	string text = (line >= 1 && static_cast<size_t>(line) <= source_lines.size()) ? source_lines[line - 1] : "";

	string message = "[DEBUG] " + std::to_string(line) + "번째 줄에서 정지";
	if (is_breakpoint_hit) message += " (breakpoint)";
	message += " → " + text;

	writer(message);
}

bool DebugSession::processCommand(const string& raw_command, const function<void(const string&)>& writer) {
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
			writer("[DEBUG] " + std::to_string(line_number) + "번째 줄에 breakpoint 설정");
		}
		return false;
	}
	if (command == "remove") {
		int line_number;
		if (iss >> line_number) {
			breakpoints.erase(line_number);
			writer("[DEBUG] " + std::to_string(line_number) + "번째 줄의 breakpoint 해제");
		}
		return false;
	}
	if (command == "breakpoints") {
		if (breakpoints.empty()) {
			writer("[DEBUG] 설정된 breakpoint가 없습니다");
		}
		else {
			string joined;
			for (int bp : breakpoints) {
				if (!joined.empty()) joined += ", ";
				joined += std::to_string(bp);
			}
			writer("[DEBUG] breakpoints: " + joined);
		}
		return false;
	}

	writer("[DEBUG] 알 수 없는 명령어: " + raw_command);
	return false;
}
