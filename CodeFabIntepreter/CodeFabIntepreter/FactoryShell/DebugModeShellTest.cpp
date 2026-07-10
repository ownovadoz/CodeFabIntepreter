#ifdef _DEBUG

#include "DebugModeShell.h"
#include "../CodeFabException.h"

#include <gmock/gmock.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using std::string;
using std::vector;
using namespace testing;

namespace {
	// DebugModeShell::enter()은 실제 CodeFabFacade::setBeforeStatementHook을 통해
	// Interpreter가 문장을 실행하기 직전마다 부르는 진짜 콜백으로 멈춘다. 그래서
	// 테스트에서도 진짜 std::cin/std::cout을 리다이렉트해 명령어를 흘려보내고
	// 출력을 캡처해야 한다. 명령어를 안 주면(빈 스트림) EOF로 즉시 continue 처리된다.
	class DebugModeShellTestFixture : public ::testing::Test {
	protected:
		void SetUp() override {
			original_cin_buffer = std::cin.rdbuf();
			original_cout_buffer = std::cout.rdbuf();
			std::cin.rdbuf(input_stream.rdbuf());
			std::cout.rdbuf(captured_output.rdbuf());
		}

		void TearDown() override {
			std::cin.rdbuf(original_cin_buffer);
			std::cout.rdbuf(original_cout_buffer);
		}

		void feedCommands(const string& commands) {
			input_stream.str(commands);
			input_stream.clear();
		}

		string output() const { return captured_output.str(); }

	private:
		std::istringstream input_stream;
		std::ostringstream captured_output;
		std::streambuf* original_cin_buffer = nullptr;
		std::streambuf* original_cout_buffer = nullptr;
	};
}

TEST_F(DebugModeShellTestFixture, EnterWithMissingFileThrowsCodeFabException) {
	DebugModeShell shell("missing.txt", [](const string&) { return false; });

	EXPECT_THROW(shell.enter(), CodeFabException);
}

TEST_F(DebugModeShellTestFixture, EnterWithNoLinesDoesNotThrow) {
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{}; });

	EXPECT_NO_THROW(shell.enter());
}

TEST_F(DebugModeShellTestFixture, EnterWithExistingFileLoadsLinesInOrder) {
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{"var a = 1;", "var b = 2;"}; });

	shell.enter();

	EXPECT_THAT(shell.getLoadedLines(), ElementsAre("var a = 1;", "var b = 2;"));
}

TEST_F(DebugModeShellTestFixture, EnterPassesFilePathToFileExistsAndReadLines) {
	string received_exists_path;
	string received_read_path;
	DebugModeShell shell(
		"script.txt",
		[&received_exists_path](const string& path) { received_exists_path = path; return true; },
		[&received_read_path](const string& path) { received_read_path = path; return vector<string>{}; });

	shell.enter();

	EXPECT_EQ(received_exists_path, "script.txt");
	EXPECT_EQ(received_read_path, "script.txt");
}

TEST_F(DebugModeShellTestFixture, PausesBeforeEachStatementAndPrintsLineText) {
	feedCommands("step\nstep\n");
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{"var a = 1;", "var b = 2;"}; });

	shell.enter();

	EXPECT_THAT(output(), HasSubstr("[DEBUG] 1번째 줄에서 정지 → var a = 1;"));
	EXPECT_THAT(output(), HasSubstr("[DEBUG] 2번째 줄에서 정지 → var b = 2;"));
}

TEST_F(DebugModeShellTestFixture, BreakThenContinueStopsAtBreakpointLine) {
	feedCommands("break 3\ncontinue\nstep\n");
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{"var a = 1;", "var b = 2;", "print a;"}; });

	shell.enter();

	EXPECT_THAT(shell.getBreakpoints(), UnorderedElementsAre(3));
	EXPECT_THAT(output(), HasSubstr("[DEBUG] 3번째 줄에서 정지 (breakpoint) → print a;"));
}

TEST_F(DebugModeShellTestFixture, SteppingOntoABreakpointLineStillShowsTheBreakpointTag) {
	// continue로 도달했을 때뿐 아니라, step으로 그냥 지나가다 breakpoint가
	// 걸린 줄에 도착해도 (breakpoint) 표시가 나와야 한다.
	feedCommands("break 2\nstep\nstep\n");
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{"var a = 1;", "var b = 2;"}; });

	shell.enter();

	EXPECT_THAT(output(), HasSubstr("[DEBUG] 2번째 줄에서 정지 (breakpoint) → var b = 2;"));
}

TEST_F(DebugModeShellTestFixture, RemoveCommandUnregistersBreakpoint) {
	feedCommands("break 2\nremove 2\nstep\nstep\n");
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{"var a = 1;", "var b = 2;"}; });

	shell.enter();

	EXPECT_THAT(shell.getBreakpoints(), IsEmpty());
}

TEST_F(DebugModeShellTestFixture, WatchCommandAddsVariableToWatchedSet) {
	feedCommands("watch a\nstep\n");
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{"var a = 1;"}; });

	shell.enter();

	EXPECT_THAT(shell.getWatchedVariables(), UnorderedElementsAre("a"));
}

TEST_F(DebugModeShellTestFixture, UnwatchCommandRemovesVariableFromWatchedSet) {
	feedCommands("watch a\nunwatch a\nstep\n");
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{"var a = 1;"}; });

	shell.enter();

	EXPECT_THAT(shell.getWatchedVariables(), IsEmpty());
}

TEST_F(DebugModeShellTestFixture, WatchedVariableValueIsAutoPrintedOnEachSubsequentPause) {
	// watch 등록 시점엔 아직 값이 없어 출력되지 않고, 그 다음 정지부터 자동 출력된다.
	feedCommands("watch a\nstep\nstep\n");
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{"var a = 1;", "a = 2;"}; });

	shell.enter();

	EXPECT_THAT(output(), HasSubstr("[WATCH] 'a' 감시 등록"));
	EXPECT_THAT(output(), HasSubstr("[WATCH] a = 1"));
}

TEST_F(DebugModeShellTestFixture, UnwatchStopsFurtherAutoPrintingOfThatVariable) {
	feedCommands("watch a\nstep\nunwatch a\nstep\n");
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{"var a = 1;", "var b = 2;", "print b;"}; });

	shell.enter();

	EXPECT_THAT(output(), HasSubstr("[WATCH] 'a' 감시 해제"));

	size_t watch_value_count = 0;
	size_t pos = 0;
	while ((pos = output().find("[WATCH] a =", pos)) != string::npos) {
		watch_value_count++;
		pos++;
	}
	EXPECT_EQ(watch_value_count, 1u);
}

TEST_F(DebugModeShellTestFixture, WatchesCommandPrintsCurrentValuesOfAllWatchedVariables) {
	feedCommands("watch a\nwatch b\nstep\nstep\nwatches\nstep\n");
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{"var a = 1;", "var b = 2;", "print a;"}; });

	shell.enter();

	EXPECT_THAT(output(), HasSubstr("[WATCH] a = 1"));
	EXPECT_THAT(output(), HasSubstr("[WATCH] b = 2"));
}

TEST_F(DebugModeShellTestFixture, InspectCommandListsLocalAndGlobalVariablesWithTypeTags) {
	feedCommands("step\nstep\ninspect\nstep\n");
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{"var g = 1;", "{ var b = 2; print b; }"}; });

	shell.enter();

	EXPECT_THAT(output(), HasSubstr("[로컬] b = 2 (Number)"));
	EXPECT_THAT(output(), HasSubstr("[전역] g = 1 (Number)"));
}

TEST_F(DebugModeShellTestFixture, NextCommandResumesExecutionLikeStep) {
	feedCommands("next\nstep\n");
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{"var a = 1;", "var b = 2;"}; });

	shell.enter();

	EXPECT_THAT(output(), HasSubstr("[DEBUG] 1번째 줄에서 정지 → var a = 1;"));
	EXPECT_THAT(output(), HasSubstr("[DEBUG] 2번째 줄에서 정지 → var b = 2;"));
}

TEST_F(DebugModeShellTestFixture, BreakpointsCommandListsCurrentlySetBreakpoints) {
	feedCommands("break 2\nbreakpoints\nstep\nstep\n");
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{"var a = 1;", "var b = 2;"}; });

	shell.enter();

	EXPECT_THAT(output(), HasSubstr("[DEBUG] breakpoints: 2"));
}

TEST_F(DebugModeShellTestFixture, BreakpointsCommandWithNoneSetPrintsEmptyMessage) {
	feedCommands("breakpoints\nstep\n");
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{"var a = 1;"}; });

	shell.enter();

	EXPECT_THAT(output(), HasSubstr("[DEBUG] 설정된 breakpoint가 없습니다"));
}

TEST_F(DebugModeShellTestFixture, UnknownCommandPrintsErrorMessageAndKeepsWaitingForInput) {
	feedCommands("bogus\nstep\n");
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{"var a = 1;"}; });

	shell.enter();

	EXPECT_THAT(output(), HasSubstr("[DEBUG] 알 수 없는 명령어: bogus"));
}

TEST_F(DebugModeShellTestFixture, InspectCommandTagsStringTypedVariableAsString) {
	// s가 정의된 이후 지점(두 번째 정지)에서 inspect해야 값이 보인다.
	feedCommands("step\ninspect\nstep\n");
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{"var s = \"hi\";", "print s;"}; });

	shell.enter();

	EXPECT_THAT(output(), HasSubstr("(String)"));
}

TEST_F(DebugModeShellTestFixture, InspectCommandTagsNilValuedVariableAsNil) {
	// CodeFab 문법은 초기화식 없는 var 선언을 허용하지 않으므로, return 없는
	// 함수 호출 결과(nil)를 담아 nil 값을 만든다.
	feedCommands("step\nstep\ninspect\nstep\n");
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{"Func f() {}", "var x = f();", "print x;"}; });

	shell.enter();

	EXPECT_THAT(output(), HasSubstr("(Nil)"));
}

TEST_F(DebugModeShellTestFixture, WatchingUndefinedVariableNamePrintsNothingButDoesNotThrow) {
	// "a"를 감시하지만 실제로는 한 번도 선언되지 않으므로, 정지 시점마다
	// 스냅샷을 끝까지 훑고도 일치하는 이름을 찾지 못한 채 조용히 넘어가야 한다.
	feedCommands("watch a\nstep\nstep\n");
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{"var b = 1;", "print b;"}; });

	EXPECT_NO_THROW(shell.enter());
	EXPECT_THAT(output(), Not(HasSubstr("[WATCH] a =")));
}

TEST_F(DebugModeShellTestFixture, MultipleStatementsOnOneLineEachPauseSeparately) {
	// 실제 Interpreter 훅으로 연결되어 있어, 한 줄에 문장이 여러 개(세미콜론으로 구분)
	// 있어도 문장 단위로 각각 멈춘다. 이전 "한 줄 = 한 Stmt" 가정으로는 불가능했던 부분이다.
	feedCommands("step\nstep\n");
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{"var a = 1; print a;"}; });

	shell.enter();

	size_t pause_count = 0;
	size_t pos = 0;
	while ((pos = output().find("번째 줄에서 정지", pos)) != string::npos) {
		pause_count++;
		pos++;
	}

	EXPECT_EQ(pause_count, 2u);
}

#endif
