#include "FileModeShell.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <utility>

using std::ifstream;
using std::move;

#ifdef _DEBUG

FileModeShell::FileModeShell(string file_path, function<bool(const string&)> file_exists, function<vector<string>(const string&)> read_lines)
	: file_path(move(file_path)), file_exists(move(file_exists)), read_lines(move(read_lines)) {}

void FileModeShell::enter() {
	if (!file_exists(file_path)) throw CodeFabException(0, "파일을 찾을 수 없습니다: '" + file_path + "'");

	runLines(read_lines(file_path));
}

#else

FileModeShell::FileModeShell(string file_path) : file_path(move(file_path)) {}

void FileModeShell::enter() {
	if (!defaultFileExists(file_path)) throw CodeFabException(0, "파일을 찾을 수 없습니다: '" + file_path + "'");

	runLines(defaultReadLines(file_path));
}

#endif

bool FileModeShell::defaultFileExists(const string& path) {
	return std::filesystem::exists(path);
}

vector<string> FileModeShell::defaultReadLines(const string& path) {
	vector<string> lines;
	ifstream file(path);
	string line;
	while (std::getline(file, line)) {
		lines.push_back(line);
	}
	return lines;
}

void FileModeShell::runLines(const vector<string>& lines) {
	int line_number = 0;
	for (const string& input_line : lines) {
		line_number++;
		code_line = input_line;
		try {
			code_fab_facade.execute(code_line);
		}
		catch (const CodeFabException& exception) {
			std::cerr << "[line " << line_number << "] Error: " << exception.getMessage() << std::endl;
			return;
		}
		catch (const std::exception& exception) {
			std::cerr << "[line " << line_number << "] [unexpected error] " << exception.what() << std::endl;
			return;
		}
		catch (...) {
			std::cerr << "[line " << line_number << "] [unexpected error] unknown exception" << std::endl;
			return;
		}
	}
}
