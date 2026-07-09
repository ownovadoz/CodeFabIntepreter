#include <gmock/gmock.h>
#include "FactoryShell/ArgumentParser.h"
#include "FactoryShell/FileModeShell.h"
#include "FactoryShell/IShellMode.h"
#include "FactoryShell/PromptShell.h"
#include "CodeFabException.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#ifndef _DEBUG
#include <windows.h>
#endif

using std::cerr;
using std::make_unique;
using std::string;
using std::unique_ptr;
using std::vector;

int main(int argc, char* argv[]) {
#ifdef _DEBUG
	::testing::InitGoogleMock(&argc, argv);
	return RUN_ALL_TESTS();
#else
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

	vector<string> args(argv + 1, argv + argc);
	ParsedArguments parsed = ArgumentParser::parse(args);

	unique_ptr<IShellMode> shell;
	switch (parsed.mode) {
	case ShellMode::Prompt:
		shell = make_unique<PromptShell>();
		break;
	case ShellMode::File:
		shell = make_unique<FileModeShell>(parsed.file_path);
		break;
	case ShellMode::Invalid:
	default:
		cerr << "사용법: CodeFabIntepreter [run <파일경로>]\n";
		return 0;
	}

	try {
		shell->enter();
	}
	catch (const CodeFabException& exception) {
		cerr << exception.what() << "\n";
	}
#endif
}
