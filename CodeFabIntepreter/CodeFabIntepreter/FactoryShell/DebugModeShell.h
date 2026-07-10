#pragma once
#include "FileBackedShell.h"

#include <set>
#include <string>
#include <vector>

using std::set;
using std::string;
using std::vector;

class DebugModeShell : public FileBackedShell {
public:
	using FileBackedShell::FileBackedShell;

	const vector<string>& getLoadedLines() const { return loaded_lines; }
	const set<int>& getBreakpoints() const { return breakpoints; }

protected:
	void afterLoad(const string& path) override;
	void beforeExecuteLine(int line_number, const string& line_text) override;

private:
	enum class Mode { Step, Next, Continue };

	static string defaultReadCommand();

	// FileBackedShell::runLines가 각 줄을 실행하기 직전에 beforeExecuteLine을
	// 통해 등록해주면, Interpreter가 실제로 문장을 실행하기 직전마다
	// onBeforeStatement를 호출한다(중첩된 블록 내부 문장 포함).
	void onBeforeStatement(int line);
	bool processCommand(const string& raw_command);
	void printPauseMessage(int line, bool is_breakpoint_hit);

	set<int> breakpoints;
	Mode mode = Mode::Step;
};
