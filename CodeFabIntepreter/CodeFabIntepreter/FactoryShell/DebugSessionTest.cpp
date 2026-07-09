#ifdef _DEBUG

#include "DebugSession.h"

#include <gmock/gmock.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using std::make_shared;
using std::move;
using std::shared_ptr;
using std::string;
using std::vector;
using namespace testing;

namespace {
	// 큐에 담긴 명령어를 순서대로 반환한다. 큐가 소진된 뒤 또 호출되면
	// beforeStatement가 불필요하게 다시 멈춰선 것이므로 테스트를 실패시킨다.
	function<string()> makeCommandQueue(vector<string> commands) {
		auto index = make_shared<size_t>(0);
		auto queue = make_shared<vector<string>>(move(commands));
		return [index, queue]() -> string {
			if (*index >= queue->size()) {
				ADD_FAILURE() << "read_command called more times than expected";
				return "continue";
			}
			return (*queue)[(*index)++];
		};
	}

	function<void(const string&)> makeOutputSink(shared_ptr<vector<string>> sink) {
		return [sink](const string& message) { sink->push_back(message); };
	}
}

TEST(DebugSessionTest, StepModePausesAndPrintsCurrentLineText) {
	auto outputs = make_shared<vector<string>>();
	DebugSession session(
		{ "var a = 3;", "var b = a + 1;" },
		makeCommandQueue({ "step" }),
		makeOutputSink(outputs));

	session.beforeStatement(1);

	EXPECT_THAT(*outputs, ElementsAre("[DEBUG] 1번째 줄에서 정지 → var a = 3;"));
}

TEST(DebugSessionTest, BreakCommandRegistersBreakpointWithoutResuming) {
	auto outputs = make_shared<vector<string>>();
	DebugSession session(
		{ "var a = 3;" },
		makeCommandQueue({ "break 7", "step" }),
		makeOutputSink(outputs));

	session.beforeStatement(1);

	EXPECT_THAT(session.getBreakpoints(), UnorderedElementsAre(7));
	EXPECT_THAT(*outputs, ElementsAre(
		"[DEBUG] 1번째 줄에서 정지 → var a = 3;",
		"[DEBUG] 7번째 줄에 breakpoint 설정"));
}

TEST(DebugSessionTest, ContinueModeSkipsLinesWithoutBreakpoints) {
	auto outputs = make_shared<vector<string>>();
	DebugSession session(
		{ "var a = 3;", "var b = 4;" },
		makeCommandQueue({ "continue" }),
		makeOutputSink(outputs));

	session.beforeStatement(1);
	// continue 모드에서 breakpoint가 없는 줄은 멈추지 않아야 하며,
	// read_command가 다시 호출되면 makeCommandQueue가 테스트를 실패시킨다.
	session.beforeStatement(2);

	EXPECT_THAT(*outputs, ElementsAre("[DEBUG] 1번째 줄에서 정지 → var a = 3;"));
}

TEST(DebugSessionTest, ContinueModeStopsAtBreakpointLine) {
	auto outputs = make_shared<vector<string>>();
	DebugSession session(
		{ "var a = 3;", "var b = 4;", "print a;" },
		makeCommandQueue({ "break 3", "continue", "step" }),
		makeOutputSink(outputs));

	session.beforeStatement(1);
	session.beforeStatement(2);
	session.beforeStatement(3);

	EXPECT_THAT(*outputs, ElementsAre(
		"[DEBUG] 1번째 줄에서 정지 → var a = 3;",
		"[DEBUG] 3번째 줄에 breakpoint 설정",
		"[DEBUG] 3번째 줄에서 정지 (breakpoint) → print a;"));
}

TEST(DebugSessionTest, RemoveCommandUnregistersBreakpoint) {
	auto outputs = make_shared<vector<string>>();
	DebugSession session(
		{ "var a = 3;" },
		makeCommandQueue({ "break 4", "remove 4", "step" }),
		makeOutputSink(outputs));

	session.beforeStatement(1);

	EXPECT_THAT(session.getBreakpoints(), IsEmpty());
}

TEST(DebugSessionTest, BreakpointsCommandListsCurrentBreakpointsInOrder) {
	auto outputs = make_shared<vector<string>>();
	DebugSession session(
		{ "var a = 3;" },
		makeCommandQueue({ "break 9", "break 2", "breakpoints", "step" }),
		makeOutputSink(outputs));

	session.beforeStatement(1);

	EXPECT_THAT(*outputs, Contains("[DEBUG] breakpoints: 2, 9"));
}

TEST(DebugSessionTest, UnknownCommandStaysPausedAndReportsError) {
	auto outputs = make_shared<vector<string>>();
	DebugSession session(
		{ "var a = 3;" },
		makeCommandQueue({ "banana", "step" }),
		makeOutputSink(outputs));

	session.beforeStatement(1);

	EXPECT_THAT(*outputs, Contains("[DEBUG] 알 수 없는 명령어: banana"));
}

TEST(DebugSessionTest, EmptyCommandIsTreatedAsContinueToAvoidHanging) {
	auto outputs = make_shared<vector<string>>();
	DebugSession session(
		{ "var a = 3;", "var b = 4;" },
		makeCommandQueue({ "" }),
		makeOutputSink(outputs));

	session.beforeStatement(1);
	// EOF로 continue 모드에 진입했다면, breakpoint 없는 다음 줄은 멈추지 않아야 한다.
	session.beforeStatement(2);

	EXPECT_THAT(*outputs, ElementsAre("[DEBUG] 1번째 줄에서 정지 → var a = 3;"));
}

#endif
