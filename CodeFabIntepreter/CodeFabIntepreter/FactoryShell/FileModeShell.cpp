#include "FileModeShell.h"

#include <filesystem>
#include <iostream>
#include <utility>

using std::cout;
using std::move;

FileModeShell::FileModeShell(function<bool(const string&)> file_exists) : file_exists(move(file_exists)) {}

void FileModeShell::run(const string& file_path) {
	if (!file_exists(file_path)) throw CodeFabException(0, "파일을 찾을 수 없습니다: '" + file_path + "'");

	cout << "[파일 모드] 실행 로직은 아직 구현되지 않았습니다: " << file_path << "\n";
}

bool FileModeShell::defaultFileExists(const string& path) {
	return std::filesystem::exists(path);
}
