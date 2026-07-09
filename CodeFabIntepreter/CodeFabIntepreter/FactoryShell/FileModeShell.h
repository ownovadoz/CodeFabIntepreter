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

class FileModeShell : public IShellMode {
public:
#ifdef _DEBUG
	explicit FileModeShell(string file_path,
		function<bool(const string&)> file_exists = defaultFileExists,
		function<vector<string>(const string&)> read_lines = defaultReadLines);
#else
	explicit FileModeShell(string file_path);
#endif

	void enter() override;
	const string& getLastLine() const { return code_line; }

private:
	static bool defaultFileExists(const string& path);
	static vector<string> defaultReadLines(const string& path);

	void runLines(const vector<string>& lines);

	string file_path;
#ifdef _DEBUG
	function<bool(const string&)> file_exists;
	function<vector<string>(const string&)> read_lines;
#endif
	string code_line;
	CodeFabFacade code_fab_facade;
};
