#ifdef _DEBUG

#include "CodeFabFacade.h"
#include "InterfaceForCodeFabTest.h"
#include "AssemblerUnit/Parser/Statement.h"
#include "CodeFabException.h"

#include <gmock/gmock.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
using std::ostringstream;
using std::string;
using std::vector;

class MockAssemblerUnit : public IAssemblerUnit {
public:
	MOCK_METHOD(vector<unique_ptr<Statement>>, assemble, (const string& code_line), (override));
};

namespace {
	vector<unique_ptr<Statement>> oneDummyStatement() {
		vector<unique_ptr<Statement>> statements;
		statements.push_back(nullptr);
		return statements;
	}
}

class MockChecker : public IChecker {
public:
	MOCK_METHOD(void, check, (const vector<unique_ptr<Statement>>& statements), (override));
};

class MockExecutor : public IExecutor {
public:
	MOCK_METHOD(void, interpret, (const vector<unique_ptr<Statement>>& statements), (override));
	MOCK_METHOD(void, setBeforeStatementHook, (function<void(int line)> hook), (override));
	MOCK_METHOD(vector<VariableSnapshot>, inspectVariables, (), (const, override));
};

class CodeFabFacadeTestFixture : public ::testing::Test {
public:
	CodeFabFacadeTestFixture() : facade(mock_assembler_unit, mock_checker, mock_executor) {}
	MockAssemblerUnit mock_assembler_unit;
	MockChecker mock_checker;
	MockExecutor mock_executor;
	CodeFabFacade facade;
};

TEST_F(CodeFabFacadeTestFixture, SequenceTest) {

	::testing::InSequence seq;
	EXPECT_CALL(mock_assembler_unit, assemble(string("var x = 10;")))
		.WillOnce(::testing::InvokeWithoutArgs(oneDummyStatement));
	EXPECT_CALL(mock_checker, check(::testing::_)).Times(1);
	EXPECT_CALL(mock_executor, interpret(::testing::_)).Times(1);

	facade.execute("var x = 10;");
}

TEST_F(CodeFabFacadeTestFixture, ExecuteCalledMultipleTimesInvokesEachDependencyPerCall) {
	EXPECT_CALL(mock_assembler_unit, assemble(::testing::_))
		.Times(2)
		.WillRepeatedly(::testing::InvokeWithoutArgs(oneDummyStatement));
	EXPECT_CALL(mock_checker, check(::testing::_)).Times(2);
	EXPECT_CALL(mock_executor, interpret(::testing::_)).Times(2);

	facade.execute("var x = 10;");
	facade.execute("var y = 20;");
}

TEST_F(CodeFabFacadeTestFixture, ExecutePropagatesCodeFabExceptionFromAssemblerUnitAndSkipsRemainingSteps) {
	EXPECT_CALL(mock_assembler_unit, assemble(::testing::_))
		.WillOnce(::testing::Throw(CodeFabException(1, "boom")));
	EXPECT_CALL(mock_checker, check(::testing::_)).Times(0);
	EXPECT_CALL(mock_executor, interpret(::testing::_)).Times(0);

	EXPECT_THROW(facade.execute("var x = 10;"), CodeFabException);
}

TEST_F(CodeFabFacadeTestFixture, ExecutePropagatesCodeFabExceptionFromCheckerAndSkipsRemainingSteps) {
	EXPECT_CALL(mock_assembler_unit, assemble(::testing::_))
		.WillOnce(::testing::InvokeWithoutArgs(oneDummyStatement));
	EXPECT_CALL(mock_checker, check(::testing::_))
		.WillOnce(::testing::Throw(CodeFabException(1, "boom")));
	EXPECT_CALL(mock_executor, interpret(::testing::_)).Times(0);

	EXPECT_THROW(facade.execute("var x = 10;"), CodeFabException);
}

TEST_F(CodeFabFacadeTestFixture, ExecutePropagatesCodeFabExceptionFromExecutor) {
	EXPECT_CALL(mock_assembler_unit, assemble(::testing::_))
		.WillOnce(::testing::InvokeWithoutArgs(oneDummyStatement));
	EXPECT_CALL(mock_checker, check(::testing::_)).Times(1);
	EXPECT_CALL(mock_executor, interpret(::testing::_))
		.WillOnce(::testing::Throw(CodeFabException(1, "boom")));

	EXPECT_THROW(facade.execute("var x = 10;"), CodeFabException);
}

TEST_F(CodeFabFacadeTestFixture, ExecutePropagatesStdExceptionFromAssemblerUnit) {
	// CodeFabException 이외의 표준 라이브러리 예외(예: std::stod의 out_of_range)도
	// 그대로 전파되어야 한다. 삼키고 계속할지/멈출지는 호출한 셸이 결정한다.
	EXPECT_CALL(mock_assembler_unit, assemble(::testing::_))
		.WillOnce(::testing::Throw(std::out_of_range("boom")));
	EXPECT_CALL(mock_checker, check(::testing::_)).Times(0);
	EXPECT_CALL(mock_executor, interpret(::testing::_)).Times(0);

	EXPECT_THROW(facade.execute("var x = 10;"), std::out_of_range);
}

TEST_F(CodeFabFacadeTestFixture, SetBeforeStatementHookForwardsToExecutor) {
	EXPECT_CALL(mock_executor, setBeforeStatementHook(::testing::_)).Times(1);

	facade.setBeforeStatementHook([](int) {});
}

TEST_F(CodeFabFacadeTestFixture, ExecutePropagatesUnknownExceptionFromAssemblerUnit) {
	// std::exception 계층에 속하지 않는 임의의 값도 그대로 전파되어야 한다.
	EXPECT_CALL(mock_assembler_unit, assemble(::testing::_))
		.WillOnce(::testing::Throw(42));
	EXPECT_CALL(mock_checker, check(::testing::_)).Times(0);
	EXPECT_CALL(mock_executor, interpret(::testing::_)).Times(0);

	EXPECT_ANY_THROW(facade.execute("var x = 10;"));
}

TEST(CodeFabFacadeDefaultConstructorTest, ExecuteDoesNotThrowWithRealDependencies) {
	CodeFabFacade facade;
	EXPECT_NO_THROW(facade.execute(""));
}

TEST(CodeFabFacadeDefaultConstructorTest, ExecutePropagatesRealCodeFabExceptionFromInvalidSyntax) {
	// "var a = ;" 는 실제 Parser가 초기화식 누락으로 CodeFabException을 던지는 입력이다.
	CodeFabFacade facade;
	EXPECT_THROW(facade.execute("var a = ;"), CodeFabException);
}

TEST(CodeFabFacadeDefaultConstructorTest, ExecutePropagatesRealCheckerSelfReferenceError) {
	// "var a = a + 1;" 은 실제 Checker가 초기화식 자기참조로 CodeFabException을 던지는 입력이다.
	CodeFabFacade facade;

	try {
		facade.execute("var a = a + 1;");
		FAIL() << "CodeFabException을 기대했지만 던져지지 않았습니다.";
	}
	catch (const CodeFabException& exception) {
		EXPECT_THAT(exception.what(), ::testing::HasSubstr("지역변수를 읽을 수 없습니다"));
	}
}

TEST(CodeFabFacadeDefaultConstructorTest, ExecutePropagatesRealCheckerDuplicateDeclarationAcrossLines) {
	// 같은 Facade 인스턴스로 여러 줄을 실행하는 REPL 시나리오에서, 전역 스코프의
	// 중복 선언은 줄이 나뉘어도 실제 Checker에 의해 검출되어야 한다.
	CodeFabFacade facade;
	facade.execute("var a = 10;");

	try {
		facade.execute("var a = 20;");
		FAIL() << "CodeFabException을 기대했지만 던져지지 않았습니다.";
	}
	catch (const CodeFabException& exception) {
		EXPECT_THAT(exception.what(), ::testing::HasSubstr("이미 해당 변수는 현재 스코프에서 사용중입니다"));
	}
}

TEST(CodeFabFacadeDefaultConstructorTest, ExecuteChecksAndRunsEveryStatementInAMultiStatementLine) {
	// 세미콜론으로 이어진 여러 문장이 한 줄에 있어도 전부 검사/실행 대상이 되어야 한다.
	// Interpreter::evaluate가 아직 리터럴 외 표현식(변수 참조 등)은 지원하지 않으므로,
	// 리터럴만 사용해 Checker/Executor 양쪽 모두가 실제로 지원하는 범위로 검증한다.
	CodeFabFacade facade;

	EXPECT_NO_THROW(facade.execute("var a = 3; print 1; { var b = 4; } print 2;"));
}

TEST(CodeFabFacadeDefaultConstructorTest, ExecutePropagatesRealCheckerDuplicateDeclarationWithinSameLine) {
	CodeFabFacade facade;

	try {
		facade.execute("var a = 3; var a = 4;");
		FAIL() << "CodeFabException을 기대했지만 던져지지 않았습니다.";
	}
	catch (const CodeFabException& exception) {
		EXPECT_THAT(exception.what(), ::testing::HasSubstr("이미 해당 변수는 현재 스코프에서 사용중입니다"));
	}
}

TEST(CodeFabFacadeDefaultConstructorTest, SetBeforeStatementHookIsInvokedForEachStatementOnRealExecution) {
	CodeFabFacade facade;
	vector<int> observed_lines;
	facade.setBeforeStatementHook([&observed_lines](int line) { observed_lines.push_back(line); });

	facade.execute("var a = 3; print 1;");

	EXPECT_THAT(observed_lines, ::testing::ElementsAre(1, 1));
}

TEST(CodeFabFacadeDefaultConstructorTest, ExecuteFunctionDefinitionAndCallPrintsReturnedValue) {
	CodeFabFacade facade;

	ostringstream captured;
	std::streambuf* original_buf = std::cout.rdbuf(captured.rdbuf());
	facade.execute("Func add(a, b) { return a + b; } print add(3, 4);");
	std::cout.rdbuf(original_buf);

	EXPECT_EQ(captured.str(), "7\n");
}

TEST(CodeFabFacadeDefaultConstructorTest, ExecuteRecursiveFunctionCallComputesFactorial) {
	CodeFabFacade facade;

	ostringstream captured;
	std::streambuf* original_buf = std::cout.rdbuf(captured.rdbuf());
	facade.execute("Func fact(n) { if (n <= 1) return 1; return n * fact(n - 1); } print fact(5);");
	std::cout.rdbuf(original_buf);

	EXPECT_EQ(captured.str(), "120\n");
}

TEST(CodeFabFacadeDefaultConstructorTest, ExecutePropagatesRealReturnOutsideFunctionError) {
	CodeFabFacade facade;

	try {
		facade.execute("return 1;");
		FAIL() << "CodeFabException을 기대했지만 던져지지 않았습니다.";
	}
	catch (const CodeFabException& exception) {
		EXPECT_THAT(exception.what(), ::testing::HasSubstr("함수 외부에서 return을 사용할 수 없습니다"));
	}
}

TEST(CodeFabFacadeDefaultConstructorTest, ExecutePropagatesRealDuplicateParameterNameError) {
	CodeFabFacade facade;

	try {
		facade.execute("Func foo(a, a) { return a; }");
		FAIL() << "CodeFabException을 기대했지만 던져지지 않았습니다.";
	}
	catch (const CodeFabException& exception) {
		EXPECT_THAT(exception.what(), ::testing::HasSubstr("이미 해당 변수는 현재 스코프에서 사용중입니다"));
	}
}

TEST(CodeFabFacadeDefaultConstructorTest, ExecutePropagatesRealCallingNonCallableValueError) {
	CodeFabFacade facade;

	try {
		facade.execute("var x = 10; x(1);");
		FAIL() << "CodeFabException을 기대했지만 던져지지 않았습니다.";
	}
	catch (const CodeFabException& exception) {
		EXPECT_THAT(exception.what(), ::testing::HasSubstr("호출할 수 없는 대상입니다"));
	}
}

TEST(CodeFabFacadeDefaultConstructorTest, ExecutePropagatesRealCallingStringVariableAsFunctionError) {
	// var x = "hello"; x();
	CodeFabFacade facade;
	facade.execute("var x = \"hello\";");

	try {
		facade.execute("x();");
		FAIL() << "CodeFabException을 기대했지만 던져지지 않았습니다.";
	}
	catch (const CodeFabException& exception) {
		EXPECT_THAT(exception.what(), ::testing::HasSubstr("호출할 수 없는 대상입니다"));
	}
}

TEST(CodeFabFacadeDefaultConstructorTest, ExecutePropagatesRealArgumentCountMismatchError) {
	CodeFabFacade facade;

	try {
		facade.execute("Func foo(a, b, c) { return a; } foo(1, 2);");
		FAIL() << "CodeFabException을 기대했지만 던져지지 않았습니다.";
	}
	catch (const CodeFabException& exception) {
		EXPECT_THAT(exception.what(), ::testing::HasSubstr("인자 개수가 일치하지 않습니다"));
	}
}

TEST(CodeFabFacadeDefaultConstructorTest, FunctionDeclaredOnOnePreviousLineCanBeCalledOnALaterLine) {
	// PromptShell의 REPL처럼 Func 선언과 호출이 서로 다른 execute() 호출(=다른 줄)에
	// 걸쳐 있어도 동작해야 한다. CodeFabFunction은 선언 시점의 FunctionStmt를 raw
	// pointer로 참조하므로, 그 문장을 조립했던 줄의 AST가 이후 줄에서도 계속
	// 살아있어야 한다(그렇지 않으면 dangling pointer로 인자 개수가 0으로 읽히는
	// 등 정의되지 않은 동작이 발생한다).
	CodeFabFacade facade;
	facade.execute("Func add(a, b) { return a + b; }");

	ostringstream captured;
	std::streambuf* original_buf = std::cout.rdbuf(captured.rdbuf());
	facade.execute("var ret = add(3, 7); print ret;");
	std::cout.rdbuf(original_buf);

	EXPECT_EQ(captured.str(), "10\n");
}

namespace {
	// print로 출력된 내용을 캡처하기 위해 std::cout의 streambuf를 잠깐 바꿔치기한다.
	string captureStdout(CodeFabFacade& facade, const string& code) {
		ostringstream captured;
		std::streambuf* original_buf = std::cout.rdbuf(captured.rdbuf());
		facade.execute(code);
		std::cout.rdbuf(original_buf);
		return captured.str();
	}
}

TEST(CodeFabFacadeClassTest, InstantiatingClassLikeAFunctionCreatesInstance) {
	CodeFabFacade facade;

	EXPECT_NO_THROW(facade.execute("Class Robot {} var r = Robot();"));
}

TEST(CodeFabFacadeClassTest, FieldsCanBeWrittenAndReadDynamically) {
	CodeFabFacade facade;
	facade.execute("Class Robot {} var r = Robot(); r.name = \"SpeedRobot\"; r.speed = 10;");

	EXPECT_EQ(captureStdout(facade, "print r.name;"), "SpeedRobot\n");
	EXPECT_EQ(captureStdout(facade, "print r.speed;"), "10\n");
}

TEST(CodeFabFacadeClassTest, FieldCanBeUpdatedFromItsOwnPreviousValue) {
	CodeFabFacade facade;
	facade.execute("Class Robot {} var r = Robot(); r.speed = 10; r.speed = r.speed + 5;");

	EXPECT_EQ(captureStdout(facade, "print r.speed;"), "15\n");
}

TEST(CodeFabFacadeClassTest, ReadingUndefinedFieldThrowsRuntimeError) {
	CodeFabFacade facade;
	facade.execute("Class Robot {} var r = Robot();");

	EXPECT_THROW(facade.execute("print r.power;"), CodeFabException);
}

TEST(CodeFabFacadeClassTest, MethodUsesThisToReadAndWriteFieldsAndCallOtherMethods) {
	CodeFabFacade facade;
	facade.execute(
		"Class Robot {"
		"  move(dist) { this.position = this.position + dist; }"
		"  report() { print this.position; }"
		"}"
		"var r = Robot(); r.position = 0; r.move(5);");

	EXPECT_EQ(captureStdout(facade, "r.report();"), "5\n");
}

TEST(CodeFabFacadeClassTest, InitIsCalledAutomaticallyOnInstantiationAndAlwaysReturnsInstance) {
	CodeFabFacade facade;
	facade.execute(
		"Class Robot { init(name, speed) { this.name = name; this.speed = speed; } }"
		"var r = Robot(\"AndOr\", 10);");

	EXPECT_EQ(captureStdout(facade, "print r.name;"), "AndOr\n");
	EXPECT_EQ(captureStdout(facade, "print r.speed;"), "10\n");
}

TEST(CodeFabFacadeClassTest, ReturnInsideInitIsRejectedByChecker) {
	CodeFabFacade facade;

	EXPECT_THROW(facade.execute("Class Robot { init() { return 5; } }"), CodeFabException);
}

TEST(CodeFabFacadeClassTest, SubclassInheritsAndCanOverrideAndCallSuperMethod) {
	CodeFabFacade facade;
	facade.execute(
		"Class Robot { move(dist) { print \"move\"; } }"
		"Class SpeedRobot : Robot {"
		"  move(dist) { Super.move(dist); print \"Speeeed!\"; }"
		"}");

	EXPECT_EQ(captureStdout(facade, "SpeedRobot().move(3);"), "move\nSpeeeed!\n");
}

TEST(CodeFabFacadeClassTest, InstanceOfIsTrueForOwnClassAndAncestorClass) {
	CodeFabFacade facade;
	facade.execute(
		"Class Robot { init(name) { this.name = name; } }"
		"Class SpeedRobot : Robot { init(name) { Super.init(name); } }"
		"var w = SpeedRobot(\"Sam\");");

	EXPECT_EQ(captureStdout(facade, "print (w instanceof SpeedRobot);"), "true\n");
	EXPECT_EQ(captureStdout(facade, "print (w instanceof Robot);"), "true\n");
}

TEST(CodeFabFacadeClassTest, InstanceOfIsFalseForUnrelatedClass) {
	CodeFabFacade facade;
	facade.execute("Class Robot {} Class Cat {} var r = Robot();");

	EXPECT_EQ(captureStdout(facade, "print (r instanceof Cat);"), "false\n");
}

TEST(CodeFabFacadeClassTest, ThisOutsideClassIsRejectedByChecker) {
	CodeFabFacade facade;

	EXPECT_THROW(facade.execute("print this;"), CodeFabException);
}

TEST(CodeFabFacadeClassTest, SuperOutsideClassIsRejectedByChecker) {
	CodeFabFacade facade;

	EXPECT_THROW(facade.execute("Super.move();"), CodeFabException);
}

TEST(CodeFabFacadeClassTest, SuperInClassWithoutParentIsRejectedByChecker) {
	CodeFabFacade facade;

	EXPECT_THROW(facade.execute("Class Robot { move() { Super.move(); } }"), CodeFabException);
}

TEST(CodeFabFacadeClassTest, SelfInheritanceIsRejectedByChecker) {
	CodeFabFacade facade;

	EXPECT_THROW(facade.execute("Class Robot : Robot {}"), CodeFabException);
}

TEST(CodeFabFacadeClassTest, InheritingFromNonClassValueThrowsRuntimeError) {
	CodeFabFacade facade;
	facade.execute("var x = 10;");

	EXPECT_THROW(facade.execute("Class Robot : x {}"), CodeFabException);
}

TEST(CodeFabFacadeClassTest, AccessingFieldOnNonInstanceThrowsRuntimeError) {
	CodeFabFacade facade;
	facade.execute("var x = \"hello\";");

	EXPECT_THROW(facade.execute("x.field = 1;"), CodeFabException);
}
#endif
