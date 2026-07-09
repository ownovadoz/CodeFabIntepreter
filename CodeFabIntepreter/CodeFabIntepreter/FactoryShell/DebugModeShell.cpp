#include "DebugModeShell.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <utility>

using std::ifstream;
using std::make_unique;
using std::move;

#ifdef _DEBUG

DebugModeShell::DebugModeShell(string file_path, function<bool(const string&)> file_exists, function<vector<string>(const string&)> read_lines)
	: file_path(move(file_path)), file_exists(move(file_exists)), read_lines(move(read_lines)) {}

void DebugModeShell::enter() {
	if (!file_exists(file_path)) throw CodeFabException(0, "파일을 찾을 수 없습니다: '" + file_path + "'");

	loaded_lines = read_lines(file_path);
	std::cout << "[DEBUG] 소스코드 로딩: " << file_path << "\n";

	runLines(loaded_lines);
}

#else

DebugModeShell::DebugModeShell(string file_path) : file_path(move(file_path)) {}

void DebugModeShell::enter() {
	if (!defaultFileExists(file_path)) throw CodeFabException(0, "파일을 찾을 수 없습니다: '" + file_path + "'");

	loaded_lines = defaultReadLines(file_path);
	std::cout << "[DEBUG] 소스코드 로딩: " << file_path << "\n";

	runLines(loaded_lines);
}

#endif

void DebugModeShell::runLines(const vector<string>& lines) {
	// Executor가 아직 Stmt 단위 콜백을 제공하지 않으므로, 현재의 "한 줄 = 한 Stmt"
	// 실행 모델을 그대로 이용해 줄 단위로 DebugSession의 콜백을 호출한다.
	// Executor가 실제 Stmt 콜백을 제공하게 되면 이 호출 지점만 옮기면 된다.
	debug_session = make_unique<DebugSession>(lines);

	int line_number = 0;
	for (const string& input_line : lines) {
		line_number++;
		debug_session->beforeStatement(line_number);

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
