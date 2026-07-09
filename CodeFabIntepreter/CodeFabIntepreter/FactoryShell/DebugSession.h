#pragma once

#include <functional>
#include <set>
#include <string>
#include <vector>

using std::function;
using std::set;
using std::string;
using std::vector;

// Executor가 아직 Stmt 단위 콜백을 제공하지 않으므로, 향후 Executor가
// 문장 실행 직전마다 beforeStatement(line)을 호출해준다고 가정하고
// stepping/breakpoint 상태 머신만 독립적으로 구현한다.
class DebugSession {
public:
#ifdef _DEBUG
	explicit DebugSession(vector<string> source_lines,
		function<string()> read_command = defaultReadCommand,
		function<void(const string&)> write_line = defaultWriteLine);
#else
	explicit DebugSession(vector<string> source_lines);
#endif

	void beforeStatement(int line);

	const set<int>& getBreakpoints() const { return breakpoints; }

private:
	enum class Mode { Step, Next, Continue };

	static string defaultReadCommand();
	static void defaultWriteLine(const string& message);

	bool processCommand(const string& raw_command, const function<void(const string&)>& writer);
	void printPauseMessage(int line, bool is_breakpoint_hit, const function<void(const string&)>& writer);

	vector<string> source_lines;
#ifdef _DEBUG
	function<string()> read_command;
	function<void(const string&)> write_line;
#endif
	set<int> breakpoints;
	Mode mode = Mode::Step;
};
