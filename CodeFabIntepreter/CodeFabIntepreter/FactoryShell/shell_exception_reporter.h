#pragma once
#include "../CodeFabException.h"

#include <iostream>

template <typename ShellAction>
void reportShellExceptions(ShellAction&& run_shell_action) {
	try {
		run_shell_action();
	}
	catch (const CodeFabException& exception) {
		std::cerr << exception.what() << std::endl;
	}
	catch (const std::exception& exception) {
		std::cerr << "[unexpected error] " << exception.what() << std::endl;
	}
	catch (...) {
		std::cerr << "[unexpected error] unknown exception" << std::endl;
	}
}
