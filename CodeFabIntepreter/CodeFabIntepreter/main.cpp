#include <gmock/gmock.h>
#include "FactoryShell/ArgumentParser.h"
#include "FactoryShell/FileModeShell.h"
#include "FactoryShell/PromptShell.h"
#include "CodeFabException.h"

#include <iostream>
#include <string>
#include <vector>

using std::cerr;
using std::string;
using std::vector;

int main(int argc, char* argv[]) {
#ifdef _DEBUG
	::testing::InitGoogleMock(&argc, argv);
	return RUN_ALL_TESTS();
#else
	vector<string> args(argv + 1, argv + argc);
	ParsedArguments parsed = ArgumentParser::parse(args);

	switch (parsed.mode) {
	case ShellMode::Prompt: {
		PromptShell shell;
		shell.runPrompt();
		break;
	}
	case ShellMode::File: {
		try {
			FileModeShell file_mode_shell;
			file_mode_shell.run(parsed.file_path);
		}
		catch (const CodeFabException& exception) {
			cerr << exception.what() << "\n";
		}
		break;
	}
	case ShellMode::Invalid:
	default:
		cerr << "사용법: CodeFabIntepreter [run <파일경로>]\n";
		break;
	}
#endif
}