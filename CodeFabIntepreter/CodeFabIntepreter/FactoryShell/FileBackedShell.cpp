#include "FileBackedShell.h"

#include <filesystem>
#include <fstream>
#include <iostream>
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

	runLines();
}

#else

FileBackedShell::FileBackedShell(string file_path) : file_path(move(file_path)) {}

void FileBackedShell::enter() {
	if (!defaultFileExists(file_path)) throw CodeFabException(0, "파일을 찾을 수 없습니다: '" + file_path + "'");

	loaded_lines = defaultReadLines(file_path);
	afterLoad(file_path);

	runLines();
}

#endif

void FileBackedShell::runLines() {
	int line_number = 0;
	for (const string& input_line : loaded_lines) {
		line_number++;
		beforeExecuteLine(line_number, input_line);

		try {
			code_fab_facade.execute(input_line);
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
