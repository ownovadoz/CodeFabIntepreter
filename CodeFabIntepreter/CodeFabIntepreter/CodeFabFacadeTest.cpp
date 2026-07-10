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

class CodeFabFacadeDefaultConstructorTestFixture : public ::testing::Test {
public:
	CodeFabFacade facade;
};

TEST_F(CodeFabFacadeDefaultConstructorTestFixture, ExecuteDoesNotThrowWithRealDependencies) {
	EXPECT_NO_THROW(facade.execute(""));
}

TEST_F(CodeFabFacadeDefaultConstructorTestFixture, ExecutePropagatesRealCodeFabExceptionFromInvalidSyntax) {
	// "var a = ;" 는 실제 Parser가 초기화식 누락으로 CodeFabException을 던지는 입력이다.
	EXPECT_THROW(facade.execute("var a = ;"), CodeFabException);
}

TEST_F(CodeFabFacadeDefaultConstructorTestFixture, ExecutePropagatesRealCheckerSelfReferenceError) {
	// "var a = a + 1;" 은 실제 Checker가 초기화식 자기참조로 CodeFabException을 던지는 입력이다.
	try {
		facade.execute("var a = a + 1;");
		FAIL() << "CodeFabException을 기대했지만 던져지지 않았습니다.";
	}
	catch (const CodeFabException& exception) {
		EXPECT_THAT(exception.what(), ::testing::HasSubstr("지역변수를 읽을 수 없습니다"));
	}
}

TEST_F(CodeFabFacadeDefaultConstructorTestFixture, ExecutePropagatesRealCheckerDuplicateDeclarationAcrossLines) {
	// 같은 Facade 인스턴스로 여러 줄을 실행하는 REPL 시나리오에서, 전역 스코프의
	// 중복 선언은 줄이 나뉘어도 실제 Checker에 의해 검출되어야 한다.
	facade.execute("var a = 10;");

	try {
		facade.execute("var a = 20;");
		FAIL() << "CodeFabException을 기대했지만 던져지지 않았습니다.";
	}
	catch (const CodeFabException& exception) {
		EXPECT_THAT(exception.what(), ::testing::HasSubstr("이미 해당 변수는 현재 스코프에서 사용중입니다"));
	}
}

TEST_F(CodeFabFacadeDefaultConstructorTestFixture, ExecuteChecksAndRunsEveryStatementInAMultiStatementLine) {
	// 세미콜론으로 이어진 여러 문장이 한 줄에 있어도 전부 검사/실행 대상이 되어야 한다.
	// Interpreter::evaluate가 아직 리터럴 외 표현식(변수 참조 등)은 지원하지 않으므로,
	// 리터럴만 사용해 Checker/Executor 양쪽 모두가 실제로 지원하는 범위로 검증한다.
	EXPECT_NO_THROW(facade.execute("var a = 3; print 1; { var b = 4; } print 2;"));
}

TEST_F(CodeFabFacadeDefaultConstructorTestFixture, ExecutePropagatesRealCheckerDuplicateDeclarationWithinSameLine) {
	try {
		facade.execute("var a = 3; var a = 4;");
		FAIL() << "CodeFabException을 기대했지만 던져지지 않았습니다.";
	}
	catch (const CodeFabException& exception) {
		EXPECT_THAT(exception.what(), ::testing::HasSubstr("이미 해당 변수는 현재 스코프에서 사용중입니다"));
	}
}

TEST_F(CodeFabFacadeDefaultConstructorTestFixture, SetBeforeStatementHookIsInvokedForEachStatementOnRealExecution) {
	vector<int> observed_lines;
	facade.setBeforeStatementHook([&observed_lines](int line) { observed_lines.push_back(line); });

	facade.execute("var a = 3; print 1;");

	EXPECT_THAT(observed_lines, ::testing::ElementsAre(1, 1));
}

TEST_F(CodeFabFacadeDefaultConstructorTestFixture, ExecuteFunctionDefinitionAndCallPrintsReturnedValue) {
	ostringstream captured;
	std::streambuf* original_buf = std::cout.rdbuf(captured.rdbuf());
	facade.execute("Func add(a, b) { return a + b; } print add(3, 4);");
	std::cout.rdbuf(original_buf);

	EXPECT_EQ(captured.str(), "7\n");
}

TEST_F(CodeFabFacadeDefaultConstructorTestFixture, ExecuteRecursiveFunctionCallComputesFactorial) {
	ostringstream captured;
	std::streambuf* original_buf = std::cout.rdbuf(captured.rdbuf());
	facade.execute("Func fact(n) { if (n <= 1) return 1; return n * fact(n - 1); } print fact(5);");
	std::cout.rdbuf(original_buf);

	EXPECT_EQ(captured.str(), "120\n");
}

TEST_F(CodeFabFacadeDefaultConstructorTestFixture, ExecutePropagatesRealReturnOutsideFunctionError) {
	try {
		facade.execute("return 1;");
		FAIL() << "CodeFabException을 기대했지만 던져지지 않았습니다.";
	}
	catch (const CodeFabException& exception) {
		EXPECT_THAT(exception.what(), ::testing::HasSubstr("함수 외부에서 return을 사용할 수 없습니다"));
	}
}

TEST_F(CodeFabFacadeDefaultConstructorTestFixture, ExecutePropagatesRealDuplicateParameterNameError) {
	try {
		facade.execute("Func foo(a, a) { return a; }");
		FAIL() << "CodeFabException을 기대했지만 던져지지 않았습니다.";
	}
	catch (const CodeFabException& exception) {
		EXPECT_THAT(exception.what(), ::testing::HasSubstr("이미 해당 변수는 현재 스코프에서 사용중입니다"));
	}
}

TEST_F(CodeFabFacadeDefaultConstructorTestFixture, ExecutePropagatesRealCallingNonCallableValueError) {
	try {
		facade.execute("var x = 10; x(1);");
		FAIL() << "CodeFabException을 기대했지만 던져지지 않았습니다.";
	}
	catch (const CodeFabException& exception) {
		EXPECT_THAT(exception.what(), ::testing::HasSubstr("호출할 수 없는 대상입니다"));
	}
}

TEST_F(CodeFabFacadeDefaultConstructorTestFixture, ExecutePropagatesRealCallingStringVariableAsFunctionError) {
	// var x = "hello"; x();
	facade.execute("var x = \"hello\";");

	try {
		facade.execute("x();");
		FAIL() << "CodeFabException을 기대했지만 던져지지 않았습니다.";
	}
	catch (const CodeFabException& exception) {
		EXPECT_THAT(exception.what(), ::testing::HasSubstr("호출할 수 없는 대상입니다"));
	}
}

TEST_F(CodeFabFacadeDefaultConstructorTestFixture, ExecutePropagatesRealArgumentCountMismatchError) {
	try {
		facade.execute("Func foo(a, b, c) { return a; } foo(1, 2);");
		FAIL() << "CodeFabException을 기대했지만 던져지지 않았습니다.";
	}
	catch (const CodeFabException& exception) {
		EXPECT_THAT(exception.what(), ::testing::HasSubstr("인자 개수가 일치하지 않습니다"));
	}
}

TEST_F(CodeFabFacadeDefaultConstructorTestFixture, FunctionDeclaredOnOnePreviousLineCanBeCalledOnALaterLine) {
	// PromptShell의 REPL처럼 Func 선언과 호출이 서로 다른 execute() 호출(=다른 줄)에
	// 걸쳐 있어도 동작해야 한다. CodeFabFunction은 선언 시점의 FunctionStmt를 raw
	// pointer로 참조하므로, 그 문장을 조립했던 줄의 AST가 이후 줄에서도 계속
	// 살아있어야 한다(그렇지 않으면 dangling pointer로 인자 개수가 0으로 읽히는
	// 등 정의되지 않은 동작이 발생한다).
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

class CodeFabFacadeClassTestFixture : public ::testing::Test {
public:
	CodeFabFacade facade;
};

TEST_F(CodeFabFacadeClassTestFixture, InstantiatingClassLikeAFunctionCreatesInstance) {
	EXPECT_NO_THROW(facade.execute("Class Robot {} var r = Robot();"));
}

TEST_F(CodeFabFacadeClassTestFixture, FieldsCanBeWrittenAndReadDynamically) {
	facade.execute("Class Robot {} var r = Robot(); r.name = \"SpeedRobot\"; r.speed = 10;");

	EXPECT_EQ(captureStdout(facade, "print r.name;"), "SpeedRobot\n");
	EXPECT_EQ(captureStdout(facade, "print r.speed;"), "10\n");
}

TEST_F(CodeFabFacadeClassTestFixture, FieldCanBeUpdatedFromItsOwnPreviousValue) {
	facade.execute("Class Robot {} var r = Robot(); r.speed = 10; r.speed = r.speed + 5;");

	EXPECT_EQ(captureStdout(facade, "print r.speed;"), "15\n");
}

TEST_F(CodeFabFacadeClassTestFixture, ReadingUndefinedFieldThrowsRuntimeError) {
	facade.execute("Class Robot {} var r = Robot();");

	EXPECT_THROW(facade.execute("print r.power;"), CodeFabException);
}

TEST_F(CodeFabFacadeClassTestFixture, MethodUsesThisToReadAndWriteFieldsAndCallOtherMethods) {
	facade.execute(
		"Class Robot {"
		"  move(dist) { this.position = this.position + dist; }"
		"  report() { print this.position; }"
		"}"
		"var r = Robot(); r.position = 0; r.move(5);");

	EXPECT_EQ(captureStdout(facade, "r.report();"), "5\n");
}

TEST_F(CodeFabFacadeClassTestFixture, InitIsCalledAutomaticallyOnInstantiationAndAlwaysReturnsInstance) {
	facade.execute(
		"Class Robot { init(name, speed) { this.name = name; this.speed = speed; } }"
		"var r = Robot(\"AndOr\", 10);");

	EXPECT_EQ(captureStdout(facade, "print r.name;"), "AndOr\n");
	EXPECT_EQ(captureStdout(facade, "print r.speed;"), "10\n");
}

TEST_F(CodeFabFacadeClassTestFixture, ReturnInsideInitIsRejectedByChecker) {
	EXPECT_THROW(facade.execute("Class Robot { init() { return 5; } }"), CodeFabException);
}

TEST_F(CodeFabFacadeClassTestFixture, SubclassInheritsAndCanOverrideAndCallSuperMethod) {
	facade.execute(
		"Class Robot { move(dist) { print \"move\"; } }"
		"Class SpeedRobot : Robot {"
		"  move(dist) { Super.move(dist); print \"Speeeed!\"; }"
		"}");

	EXPECT_EQ(captureStdout(facade, "SpeedRobot().move(3);"), "move\nSpeeeed!\n");
}

TEST_F(CodeFabFacadeClassTestFixture, InstanceOfIsTrueForOwnClassAndAncestorClass) {
	facade.execute(
		"Class Robot { init(name) { this.name = name; } }"
		"Class SpeedRobot : Robot { init(name) { Super.init(name); } }"
		"var w = SpeedRobot(\"Sam\");");

	EXPECT_EQ(captureStdout(facade, "print (w instanceof SpeedRobot);"), "true\n");
	EXPECT_EQ(captureStdout(facade, "print (w instanceof Robot);"), "true\n");
}

TEST_F(CodeFabFacadeClassTestFixture, InstanceOfIsFalseForUnrelatedClass) {
	facade.execute("Class Robot {} Class Cat {} var r = Robot();");

	EXPECT_EQ(captureStdout(facade, "print (r instanceof Cat);"), "false\n");
}

TEST_F(CodeFabFacadeClassTestFixture, ThisOutsideClassIsRejectedByChecker) {
	EXPECT_THROW(facade.execute("print this;"), CodeFabException);
}

TEST_F(CodeFabFacadeClassTestFixture, SuperOutsideClassIsRejectedByChecker) {
	EXPECT_THROW(facade.execute("Super.move();"), CodeFabException);
}

TEST_F(CodeFabFacadeClassTestFixture, SuperInClassWithoutParentIsRejectedByChecker) {
	EXPECT_THROW(facade.execute("Class Robot { move() { Super.move(); } }"), CodeFabException);
}

TEST_F(CodeFabFacadeClassTestFixture, SelfInheritanceIsRejectedByChecker) {
	EXPECT_THROW(facade.execute("Class Robot : Robot {}"), CodeFabException);
}

TEST_F(CodeFabFacadeClassTestFixture, InheritingFromNonClassValueThrowsRuntimeError) {
	facade.execute("var x = 10;");

	EXPECT_THROW(facade.execute("Class Robot : x {}"), CodeFabException);
}

TEST_F(CodeFabFacadeClassTestFixture, AccessingFieldOnNonInstanceThrowsRuntimeError) {
	facade.execute("var x = \"hello\";");

	EXPECT_THROW(facade.execute("x.field = 1;"), CodeFabException);
}

TEST_F(CodeFabFacadeDefaultConstructorTestFixture, ExecuteArrayCreationAndIndexReadWriteSucceeds) {
	// var arr = Array(3); arr[0] = 10; arr[1] = 20; arr[2] = 30; print arr[0];
	ostringstream captured;
	std::streambuf* original_buf = std::cout.rdbuf(captured.rdbuf());
	facade.execute("var arr = Array(3); arr[0] = 10; arr[1] = 20; arr[2] = 30; print arr[0];");
	std::cout.rdbuf(original_buf);

	EXPECT_EQ(captured.str(), "10\n");
}

TEST_F(CodeFabFacadeDefaultConstructorTestFixture, ExecuteArrayIndexWithComputedExpressionSucceeds) {
	// var arr = Array(3); var i = 2; arr[i - 1] = 7; print arr[1];
	ostringstream captured;
	std::streambuf* original_buf = std::cout.rdbuf(captured.rdbuf());
	facade.execute("var arr = Array(3); var i = 2; arr[i - 1] = 7; print arr[1];");
	std::cout.rdbuf(original_buf);

	EXPECT_EQ(captured.str(), "7\n");
}

TEST_F(CodeFabFacadeDefaultConstructorTestFixture, ExecuteArrayCreatedOnOneLineCanBeIndexedOnALaterLine) {
	// REPL 여러 줄에 걸쳐도 Array가 정상 동작해야 한다 (이번에 리포트된 실제 버그 시나리오).
	facade.execute("var b = Array(3);");

	ostringstream captured;
	std::streambuf* original_buf = std::cout.rdbuf(captured.rdbuf());
	facade.execute("b[0] = 1; print b[0];");
	std::cout.rdbuf(original_buf);

	EXPECT_EQ(captured.str(), "1\n");
}

TEST_F(CodeFabFacadeDefaultConstructorTestFixture, ExecutePropagatesRealArrayIndexOutOfRangeError) {
	facade.execute("var arr = Array(3);");

	try {
		facade.execute("print arr[5];");
		FAIL() << "CodeFabException을 기대했지만 던져지지 않았습니다.";
	}
	catch (const CodeFabException& exception) {
		EXPECT_THAT(exception.what(), ::testing::HasSubstr("배열 범위를 벗어났습니다"));
	}
}

TEST_F(CodeFabFacadeDefaultConstructorTestFixture, ExecutePropagatesRealArrayIndexTypeError) {
	facade.execute("var arr = Array(3);");

	try {
		facade.execute("print arr[\"hello\"];");
		FAIL() << "CodeFabException을 기대했지만 던져지지 않았습니다.";
	}
	catch (const CodeFabException& exception) {
		EXPECT_THAT(exception.what(), ::testing::HasSubstr("배열 인덱스는 숫자여야 합니다"));
	}
}

TEST_F(CodeFabFacadeDefaultConstructorTestFixture, ExecutePropagatesRealNonArrayIndexingError) {
	facade.execute("var x = 10;");

	try {
		facade.execute("print x[0];");
		FAIL() << "CodeFabException을 기대했지만 던져지지 않았습니다.";
	}
	catch (const CodeFabException& exception) {
		EXPECT_THAT(exception.what(), ::testing::HasSubstr("배열이 아닌 값에는 인덱스로 접근할 수 없습니다"));
	}
}

TEST_F(CodeFabFacadeDefaultConstructorTestFixture, ExecutePropagatesRealArraySizeTypeError) {
	try {
		facade.execute("var brr = Array(\"hi\");");
		FAIL() << "CodeFabException을 기대했지만 던져지지 않았습니다.";
	}
	catch (const CodeFabException& exception) {
		EXPECT_THAT(exception.what(), ::testing::HasSubstr("배열 크기는 숫자여야 합니다"));
	}
}

TEST_F(CodeFabFacadeDefaultConstructorTestFixture, LiteralOnlyExpressionStillEvaluatesCorrectlyAfterConstantFolding) {
	ostringstream captured;
	std::streambuf* original_buf = std::cout.rdbuf(captured.rdbuf());
	facade.execute("print 1 + 2 * 3 - (4 / 2);");
	std::cout.rdbuf(original_buf);

	EXPECT_EQ(captured.str(), "5\n");
}

TEST_F(CodeFabFacadeDefaultConstructorTestFixture, LiteralOnlyDivisionByZeroStillThrowsAtRuntimeRatherThanBeingFolded) {
	// 상수 폴딩이 0으로 나누기 같은 실행 오류를 미리 접어서 삼켜버리면 안 되고,
	// 실제 실행 시점에 원래와 동일한 예외가 나야 한다.
	try {
		facade.execute("print 1 / 0;");
		FAIL() << "CodeFabException을 기대했지만 던져지지 않았습니다.";
	}
	catch (const CodeFabException& exception) {
		EXPECT_THAT(exception.what(), ::testing::HasSubstr("0으로 나눌 수 없습니다"));
	}
}

TEST_F(CodeFabFacadeDefaultConstructorTestFixture, ClosureCapturesVariableVisibleAtDeclarationRatherThanAtCallTime) {
	// 정적 바인딩(Resolver)이 없다면 이름만으로 enclosing 체인을 훑기 때문에,
	// showA() 선언 이후 블록 안에 새로 선언된 "block" a를 두 번째 호출에서
	// 잘못 찾아 "global\nblock\n"을 출력하게 된다. 정적 바인딩은 showA가
	// 선언되던 시점의 어휘적 스코프(바깥의 전역 a)에 고정되므로 두 번 모두
	// "global"이 출력되어야 한다.
	ostringstream captured;
	std::streambuf* original_buf = std::cout.rdbuf(captured.rdbuf());
	facade.execute(
		"var a = \"global\";"
		"{"
		"  Func showA() { print a; }"
		"  showA();"
		"  var a = \"block\";"
		"  showA();"
		"}");
	std::cout.rdbuf(original_buf);

	EXPECT_EQ(captured.str(), "global\nglobal\n");
}

TEST_F(CodeFabFacadeDefaultConstructorTestFixture, ForLoopIncrementReassignsResolvedLoopVariableAcrossIterations) {
	// for의 증감식(i = i + 1)은 반복문 자신의 스코프에서 선언된 지역 변수를
	// 다시 대입하는 것이므로, Resolver가 계산해둔 거리로 Environment::assignAt을
	// 거쳐야 한다(정적 바인딩이 도입되기 전까지 실제 반복이 있는 for 문이 실제
	// 파이프라인으로 검증된 적이 없었다).
	ostringstream captured;
	std::streambuf* original_buf = std::cout.rdbuf(captured.rdbuf());
	facade.execute("for (var i = 0; i < 3; i = i + 1) { print i; }");
	std::cout.rdbuf(original_buf);

	EXPECT_EQ(captured.str(), "0\n1\n2\n");
}
#endif
