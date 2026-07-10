#include "FileBackedShell.h"
#include "shell_exception_reporter.h"

#include <filesystem>
#include <fstream>
#include <utility>

using std::ifstream;
using std::move;

#ifdef _DEBUG

FileBackedShell::FileBackedShell(string file_path, function<bool(const string&)> file_exists, function<vector<string>(const string&)> read_lines)
	: file_path(move(file_path)), file_exists(move(file_exists)), read_lines(move(read_lines)) {}

void FileBackedShell::enter() {
	if (!file_exists(file_path)) throw CodeFabException(0, "파일을 찾을 수 없습니다: '" + file_path + "'");

	loaded_lines = read_lines(file_path);
	afterLoad(file_path);
	beforeExecute();

	reportShellExceptions([&] { code_fab_facade.execute(joinLoadedLines()); });
}

#else

FileBackedShell::FileBackedShell(string file_path) : file_path(move(file_path)) {}

void FileBackedShell::enter() {
	if (!defaultFileExists(file_path)) throw CodeFabException(0, "파일을 찾을 수 없습니다: '" + file_path + "'");

	loaded_lines = defaultReadLines(file_path);
	afterLoad(file_path);
	beforeExecute();

	reportShellExceptions([&] { code_fab_facade.execute(joinLoadedLines()); });
}

#endif

bool FileBackedShell::defaultFileExists(const string& path) {
	return std::filesystem::exists(path);
}

vector<string> FileBackedShell::defaultReadLines(const string& path) {
	vector<string> lines;
	ifstream file(path);
	string line;
	while (std::getline(file, line)) {
		lines.push_back(line);
	}
	return lines;
}

string FileBackedShell::joinLoadedLines() const {
	string source;
	for (const string& line : loaded_lines) {
		source += line;
		source += '\n';
	}
	return source;
}
