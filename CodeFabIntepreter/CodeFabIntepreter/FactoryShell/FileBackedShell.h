#pragma once
#include "IShellMode.h"
#include "../CodeFabException.h"
#include "../CodeFabFacade.h"

#include <string>
#include <vector>

#ifdef _DEBUG
#include <functional>
#endif

using std::string;
using std::vector;

#ifdef _DEBUG
using std::function;
#endif

// 파일에서 소스를 읽어 줄 단위로 실행하는 셸들의 공통 동작(Template Method).
// 파일 로딩, 존재 검사, 줄 단위 실행과 에러 처리는 이 클래스가 담당하고,
// 하위 클래스는 로딩 직후(afterLoad)와 각 줄 실행 직전(beforeExecuteLine)에만
// 서로 다르게 개입한다.
class FileBackedShell : public IShellMode {
public:
#ifdef _DEBUG
	explicit FileBackedShell(string file_path,
		function<bool(const string&)> file_exists = defaultFileExists,
		function<vector<string>(const string&)> read_lines = defaultReadLines);
#else
	explicit FileBackedShell(string file_path);
#endif

	void enter() override;

protected:
	virtual void afterLoad(const string& path) {}
	virtual void beforeExecuteLine(int line_number, const string& line_text) {}

	vector<string> loaded_lines;
	CodeFabFacade code_fab_facade;

private:
	static bool defaultFileExists(const string& path);
	static vector<string> defaultReadLines(const string& path);

	void runLines();

	string file_path;
#ifdef _DEBUG
	function<bool(const string&)> file_exists;
	function<vector<string>(const string&)> read_lines;
#endif
};
