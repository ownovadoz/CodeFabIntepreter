#pragma once
#include "IShellMode.h"
#include "../CodeFabException.h"
#include "../CodeFabFacade.h"

#include <set>
#include <string>
#include <vector>

#ifdef _DEBUG
#include <functional>
#endif

using std::set;
using std::string;
using std::vector;

#ifdef _DEBUG
using std::function;
#endif

class DebugModeShell : public IShellMode {
public:
#ifdef _DEBUG
	explicit DebugModeShell(string file_path,
		function<bool(const string&)> file_exists = defaultFileExists,
		function<vector<string>(const string&)> read_lines = defaultReadLines);
#else
	explicit DebugModeShell(string file_path);
#endif

	void enter() override;
	const vector<string>& getLoadedLines() const { return loaded_lines; }
	const set<int>& getBreakpoints() const { return breakpoints; }

private:
	enum class Mode { Step, Next, Continue };

	static bool defaultFileExists(const string& path);
	static vector<string> defaultReadLines(const string& path);
	static string defaultReadCommand();

	void runLines(const vector<string>& lines);

	// CodeFabFacade::setBeforeStatementHook로 등록되어, Interpreter가 실제로
	// 문장을 실행하기 직전마다 호출된다(중첩된 블록 내부 문장 포함).
	void onBeforeStatement(int line);
	bool processCommand(const string& raw_command);
	void printPauseMessage(int line, bool is_breakpoint_hit);

	string file_path;
#ifdef _DEBUG
	function<bool(const string&)> file_exists;
	function<vector<string>(const string&)> read_lines;
#endif
	vector<string> loaded_lines;
	set<int> breakpoints;
	Mode mode = Mode::Step;
	CodeFabFacade code_fab_facade;
};
