#include "ArgumentParser.h"

ParsedArguments ArgumentParser::parse(const vector<string>& args) {
	if (args.empty()) return { ShellMode::Prompt, "" };
	if (args.size() == 2 && args[0] == "run") return { ShellMode::File, args[1] };
	if (args.size() == 2 && args[0] == "debug") return { ShellMode::Debug, args[1] };
	return { ShellMode::Invalid, "" };
}
