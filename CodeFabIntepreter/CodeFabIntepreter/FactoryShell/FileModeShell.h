#pragma once
#include "IShellMode.h"
#include "../CodeFabException.h"
#include "../CodeFabFacade.h"

#include <string>

#ifdef _DEBUG
#include <functional>
#endif

using std::string;

#ifdef _DEBUG
using std::function;
#endif

class FileModeShell : public IShellMode {
public:
#ifdef _DEBUG
	explicit FileModeShell(string file_path,
		function<bool(const string&)> file_exists = defaultFileExists,
		function<string(const string&)> read_source = defaultReadSource);
#else
	explicit FileModeShell(string file_path);
#endif

	void enter() override;

private:
	static bool defaultFileExists(const string& path);
	static string defaultReadSource(const string& path);

	void runSource(const string& source);

	string file_path;
#ifdef _DEBUG
	function<bool(const string&)> file_exists;
	function<string(const string&)> read_source;
#endif
	CodeFabFacade code_fab_facade;
};
