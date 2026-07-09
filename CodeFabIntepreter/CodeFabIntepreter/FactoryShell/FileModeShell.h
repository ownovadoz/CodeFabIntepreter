#pragma once
#include "../CodeFabException.h"
#include "../CodeFabFacade.h"

#include <functional>
#include <string>

using std::function;
using std::string;

class FileModeShell {
public:
	explicit FileModeShell(function<bool(const string&)> file_exists = defaultFileExists);

	void run(const string& file_path);
	const string& getLastLine() const { return code_line; }

private:
	static bool defaultFileExists(const string& path);

	function<bool(const string&)> file_exists;
	string code_line;
	CodeFabFacade code_fab_facade;
};
