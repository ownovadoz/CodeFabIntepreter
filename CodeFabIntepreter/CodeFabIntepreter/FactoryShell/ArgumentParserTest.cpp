#include "ArgumentParser.h"

#include <gmock/gmock.h>
#include <string>
#include <vector>

using std::string;
using std::vector;
using namespace testing;

TEST(ArgumentParserTest, NoArgumentsSelectsPromptMode) {
	ParsedArguments result = ArgumentParser::parse({});

	EXPECT_EQ(result.mode, ShellMode::Prompt);
}

TEST(ArgumentParserTest, RunWithFilePathSelectsFileMode) {
	ParsedArguments result = ArgumentParser::parse({ "run", "script.txt" });

	EXPECT_EQ(result.mode, ShellMode::File);
	EXPECT_EQ(result.file_path, "script.txt");
}

TEST(ArgumentParserTest, DebugWithFilePathSelectsDebugMode) {
	ParsedArguments result = ArgumentParser::parse({ "debug", "script.txt" });

	EXPECT_EQ(result.mode, ShellMode::Debug);
	EXPECT_EQ(result.file_path, "script.txt");
}