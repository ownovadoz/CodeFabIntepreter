#pragma once
#include "IShellMode.h"
#include "../CodeFabException.h"
#include "../CodeFabFacade.h"

#include <string>
#include <vector>

#ifdef _DEBUG
#include <functional>
#endif

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

private:
	static bool defaultFileExists(const string& path);
	static vector<string> defaultReadLines(const string& path);

	string file_path;
#ifdef _DEBUG
	function<bool(const string&)> file_exists;
	function<vector<string>(const string&)> read_lines;
#endif
	vector<string> loaded_lines;
	CodeFabFacade code_fab_facade;
};
