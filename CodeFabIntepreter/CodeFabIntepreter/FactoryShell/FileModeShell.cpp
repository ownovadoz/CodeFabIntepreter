#include "FileModeShell.h"
#include "shell_exception_reporter.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <utility>

using std::ifstream;
using std::move;
using std::ostringstream;

#ifdef _DEBUG

FileModeShell::FileModeShell(string file_path, function<bool(const string&)> file_exists, function<string(const string&)> read_source)
	: file_path(move(file_path)), file_exists(move(file_exists)), read_source(move(read_source)) {}

void FileModeShell::enter() {
	if (!file_exists(file_path)) throw CodeFabException(0, "파일을 찾을 수 없습니다: '" + file_path + "'");

	runSource(read_source(file_path));
}

#else

FileModeShell::FileModeShell(string file_path) : file_path(move(file_path)) {}

void FileModeShell::enter() {
	if (!defaultFileExists(file_path)) throw CodeFabException(0, "파일을 찾을 수 없습니다: '" + file_path + "'");

	runSource(defaultReadSource(file_path));
}

#endif

bool FileModeShell::defaultFileExists(const string& path) {
	return std::filesystem::exists(path);
}

string FileModeShell::defaultReadSource(const string& path) {
	ifstream file(path);
	ostringstream source;
	source << file.rdbuf();
	return source.str();
}

void FileModeShell::runSource(const string& source) {
	reportShellExceptions([&] { code_fab_facade.execute(source); });
}
