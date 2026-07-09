#pragma once
#include "ShellMode.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

struct ParsedArguments {
	ShellMode mode = ShellMode::Invalid;
	string file_path;
};

class ArgumentParser {
public:
	static ParsedArguments parse(const vector<string>& args);
};
