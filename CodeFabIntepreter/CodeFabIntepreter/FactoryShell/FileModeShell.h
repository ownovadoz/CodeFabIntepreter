#pragma once
#include "FileBackedShell.h"

#include <string>

#ifdef _DEBUG
#include <functional>
#endif

using std::string;

#ifdef _DEBUG
using std::function;
#endif

class FileModeShell : public FileBackedShell {
public:
#ifdef _DEBUG
	explicit FileModeShell(string file_path,
		function<bool(const string&)> file_exists = defaultFileExists,
		function<string(const string&)> read_source = defaultReadSource);
#else
	using FileBackedShell::FileBackedShell;
#endif

private:
#ifdef _DEBUG
	static string defaultReadSource(const string& path);
#endif
};
