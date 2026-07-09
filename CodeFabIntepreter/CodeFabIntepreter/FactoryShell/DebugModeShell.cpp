#include "DebugModeShell.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <utility>

using std::ifstream;
using std::move;

#ifdef _DEBUG

DebugModeShell::DebugModeShell(string file_path, function<bool(const string&)> file_exists, function<vector<string>(const string&)> read_lines)
	: file_path(move(file_path)), file_exists(move(file_exists)), read_lines(move(read_lines)) {}

void DebugModeShell::enter() {
	if (!file_exists(file_path)) throw CodeFabException(0, "파일을 찾을 수 없습니다: '" + file_path + "'");

	loaded_lines = read_lines(file_path);
	std::cout << "[DEBUG] 소스코드 로딩: " << file_path << "\n";
}

#else

DebugModeShell::DebugModeShell(string file_path) : file_path(move(file_path)) {}

void DebugModeShell::enter() {
	if (!defaultFileExists(file_path)) throw CodeFabException(0, "파일을 찾을 수 없습니다: '" + file_path + "'");

	loaded_lines = defaultReadLines(file_path);
	std::cout << "[DEBUG] 소스코드 로딩: " << file_path << "\n";
}

#endif

bool DebugModeShell::defaultFileExists(const string& path) {
	return std::filesystem::exists(path);
}

vector<string> DebugModeShell::defaultReadLines(const string& path) {
	vector<string> lines;
	ifstream file(path);
	string line;
	while (std::getline(file, line)) {
		lines.push_back(line);
	}
	return lines;
}
