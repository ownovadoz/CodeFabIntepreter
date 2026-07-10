#pragma once
#include "DebugCommand.h"
#include "FileBackedShell.h"

#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

using std::set;
using std::string;
using std::unique_ptr;
using std::unordered_map;
using std::vector;

class DebugModeShell : public FileBackedShell {
public:
	using FileBackedShell::FileBackedShell;

	const vector<string>& getLoadedLines() const { return loaded_lines; }
	const set<int>& getBreakpoints() const { return breakpoints; }
	const set<string>& getWatchedVariables() const { return watched_variables; }

	// 아래는 DebugCommand 각 구현체(Command 패턴의 Command 역할)가 호출하는
	// 리시버 메서드다. DebugModeShell 자신은 Command 객체를 몰라도 되도록,
	// 상태 변경/출력 책임만 여기 모아둔다.
	void enterStepMode() { mode = Mode::Step; }
	void enterNextMode() { mode = Mode::Next; }
	void enterContinueMode() { mode = Mode::Continue; }
	void addBreakpoint(int line);
	void removeBreakpoint(int line);
	void printBreakpoints();
	void addWatch(const string& name);
	void removeWatch(const string& name);
	void printWatchedVariables();
	void printInspect();

protected:
	void afterLoad(const string& path) override;
	void beforeExecute() override;

private:
	enum class Mode { Step, Next, Continue };

	static string defaultReadCommand();
	static unordered_map<string, unique_ptr<DebugCommand>> createCommandTable();

	// FileBackedShell::enter가 전체 소스를 실행하기 직전에 beforeExecute를 통해
	// 등록해주면, Interpreter가 실제로 문장을 실행하기 직전마다(중첩된 블록 내부
	// 문장 포함) 실제 절대 줄 번호와 함께 onBeforeStatement를 호출한다.
	void onBeforeStatement(int line);
	bool processCommand(const string& raw_command);
	void printPauseMessage(int line, bool is_breakpoint_hit);

	set<int> breakpoints;
	set<string> watched_variables;
	Mode mode = Mode::Step;
	unordered_map<string, unique_ptr<DebugCommand>> commands = createCommandTable();
};
