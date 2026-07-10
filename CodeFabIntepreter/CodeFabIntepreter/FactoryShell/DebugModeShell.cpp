#include "DebugModeShell.h"

#include "../AssemblerUnit/Tokenizer/Value.h"

#include <iostream>
#include <sstream>
#include <utility>
#include <variant>

using std::cin;
using std::cout;
using std::istringstream;
using std::make_unique;

namespace {
	string valueTypeName(const Value& value) {
		if (std::holds_alternative<bool>(value)) return "Boolean";
		if (std::holds_alternative<double>(value)) return "Number";
		if (std::holds_alternative<string>(value)) return "String";
		return "Nil";
	}

	void printSection(const string& tag, const vector<VariableSnapshot>& snapshot, bool is_global) {
		for (const auto& entry : snapshot) {
			if (entry.is_global != is_global) continue;
			cout << tag << " " << entry.name << " = " << stringify(entry.value) << " (" << valueTypeName(entry.value) << ")\n";
		}
	}
}

void DebugModeShell::afterLoad(const string& path) {
	cout << "[DEBUG] 소스코드 로딩: " << path << "\n";
}

void DebugModeShell::beforeExecute() {
	// 전체 소스를 한 번에 실행하므로, Interpreter가 알려주는 줄 번호가 이미
	// 파일 기준 절대 줄 번호다.
	code_fab_facade.setBeforeStatementHook([this](int line) { onBeforeStatement(line); });
}

void DebugModeShell::onBeforeStatement(int line) {
	bool is_breakpoint = breakpoints.count(line) > 0;
	bool should_pause = mode != Mode::Continue || is_breakpoint;
	if (!should_pause) return;

	printPauseMessage(line, is_breakpoint);
	printWatchedVariables();
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

unordered_map<string, unique_ptr<DebugCommand>> DebugModeShell::createCommandTable() {
	unordered_map<string, unique_ptr<DebugCommand>> table;
	table["step"] = make_unique<StepCommand>();
	table["next"] = make_unique<NextCommand>();
	table["continue"] = make_unique<ContinueCommand>();
	table["break"] = make_unique<BreakCommand>();
	table["remove"] = make_unique<RemoveBreakpointCommand>();
	table["breakpoints"] = make_unique<BreakpointsCommand>();
	table["watch"] = make_unique<WatchCommand>();
	table["unwatch"] = make_unique<UnwatchCommand>();
	table["watches"] = make_unique<WatchesCommand>();
	table["inspect"] = make_unique<InspectCommand>();
	return table;
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

	auto found = commands.find(command);
	if (found == commands.end()) {
		cout << "[DEBUG] 알 수 없는 명령어: " << raw_command << "\n";
		return false;
	}

	return found->second->execute(*this, iss);
}

void DebugModeShell::addBreakpoint(int line) {
	breakpoints.insert(line);
	cout << "[DEBUG] " << line << "번째 줄에 breakpoint 설정\n";
}

void DebugModeShell::removeBreakpoint(int line) {
	breakpoints.erase(line);
	cout << "[DEBUG] " << line << "번째 줄의 breakpoint 해제\n";
}

void DebugModeShell::printBreakpoints() {
	if (breakpoints.empty()) {
		cout << "[DEBUG] 설정된 breakpoint가 없습니다\n";
		return;
	}

	string joined;
	for (int bp : breakpoints) {
		if (!joined.empty()) joined += ", ";
		joined += std::to_string(bp);
	}
	cout << "[DEBUG] breakpoints: " << joined << "\n";
}

void DebugModeShell::addWatch(const string& name) {
	watched_variables.insert(name);
	cout << "[WATCH] '" << name << "' 감시 등록\n";
}

void DebugModeShell::removeWatch(const string& name) {
	watched_variables.erase(name);
	cout << "[WATCH] '" << name << "' 감시 해제\n";
}

void DebugModeShell::printWatchedVariables() {
	if (watched_variables.empty()) return;

	vector<VariableSnapshot> snapshot = code_fab_facade.inspectVariables();
	for (const string& name : watched_variables) {
		for (const auto& entry : snapshot) {
			if (entry.name != name) continue;
			cout << "[WATCH] " << name << " = " << stringify(entry.value) << "\n";
			break;
		}
	}
}

void DebugModeShell::printInspect() {
	cout << "—— 현재 스코프 변수 ————————————————\n";

	vector<VariableSnapshot> snapshot = code_fab_facade.inspectVariables();
	printSection("[로컬]", snapshot, false);
	printSection("[전역]", snapshot, true);
}
