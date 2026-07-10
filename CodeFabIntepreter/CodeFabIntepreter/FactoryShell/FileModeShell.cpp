#include "FileModeShell.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <utility>

using std::ifstream;
using std::move;
using std::ostringstream;

#ifdef _DEBUG

namespace {
	// FileModeShell은 소스를 통째로 문자열로 주입받지만, FileBackedShell은 화면
	// 표시(및 DebugModeShell의 재사용)를 위해 줄 배열을 필요로 한다. 두 표현을
	// 일치시키기 위해 '\n' 기준으로 다시 나눠 FileBackedShell에 그대로 넘긴다.
	vector<string> splitIntoLines(const string& source) {
		vector<string> lines;
		size_t start = 0;

		while (start < source.size()) {
			size_t newline = source.find('\n', start);
			if (newline == string::npos) {
				lines.push_back(source.substr(start));
				break;
			}
			lines.push_back(source.substr(start, newline - start));
			start = newline + 1;
		}

		return lines;
	}

	function<vector<string>(const string&)> asLineReader(function<string(const string&)> read_source) {
		return [read_source = move(read_source)](const string& path) {
			return splitIntoLines(read_source(path));
		};
	}
}

FileModeShell::FileModeShell(string file_path, function<bool(const string&)> file_exists, function<string(const string&)> read_source)
	: FileBackedShell(move(file_path), move(file_exists), asLineReader(move(read_source))) {}

string FileModeShell::defaultReadSource(const string& path) {
	ifstream file(path);
	ostringstream source;
	source << file.rdbuf();
	return source.str();
}

#endif
