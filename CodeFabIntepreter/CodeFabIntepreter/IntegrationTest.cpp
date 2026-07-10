#ifdef _DEBUG

// 개별 유닛(Checker/Interpreter/Resolver/ConstantFolder/...)은 각자의 *Test.cpp가
// 이미 촘촘히 검증한다. 이 파일은 그 유닛들을 조합했을 때도 여전히 맞물려
// 동작하는지를 확인하는 통합 테스트로, 한 프로그램 안에 여러 기능(클래스 상속,
// 배열 필드, 재귀 함수, 클로저/정적 바인딩, 상수 폴딩, import)을 함께 담아
// assemble → check → interpret 전체 파이프라인을 실제로 돌려본다.

#include "CodeFabFacade.h"
#include "AssemblerUnit/AssemblerUnit.h"
#include "CheckerUnit/Checker.h"
#include "CodeFabException.h"
#include "ExecutorUnit/Interpreter.h"

#include <gmock/gmock.h>

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

using std::ostringstream;
using std::string;
using std::unordered_map;

namespace {
    string captureStdout(CodeFabFacade& facade, const string& code) {
        ostringstream captured;
        std::streambuf* original_buf = std::cout.rdbuf(captured.rdbuf());
        facade.execute(code);
        std::cout.rdbuf(original_buf);
        return captured.str();
    }

    // import를 실제로 exercise하려면 파일을 읽는 Interpreter가 필요한데,
    // CodeFabFacade는 파일 시스템을 주입할 방법이 없다(항상 실제 디스크를
    // 읽는 Interpreter를 스스로 만든다). 그래서 이 통합 테스트에서는
    // CodeFabFacade가 내부적으로 하는 일(assemble → check → interpret)을
    // 가짜 파일 시스템을 주입한 Interpreter로 직접 재현한다 - 프로덕션
    // 파이프라인과 실행 순서는 동일하되, import가 실제 디스크 대신 메모리
    // 안의 가짜 파일을 읽도록 한 것만 다르다.
    class FakeFileSystem {
    public:
        void addFile(const string& path, const string& source) {
            files[path] = source;
        }

        bool exists(const string& path) const {
            return files.find(path) != files.end();
        }

        string read(const string& path) const {
            return files.at(path);
        }

    private:
        unordered_map<string, string> files;
    };

    class TestPipeline {
    public:
        FakeFileSystem file_system;
        AssemblerUnit assembler;
        Checker checker;
        Interpreter interpreter{
            [this](const string& path) { return file_system.exists(path); },
            [this](const string& path) { return file_system.read(path); }
        };

        string run(const string& code) {
            auto statements = assembler.assemble(code);
            checker.check(statements);

            ostringstream captured;
            std::streambuf* original_buf = std::cout.rdbuf(captured.rdbuf());
            interpreter.interpret(statements);
            std::cout.rdbuf(original_buf);
            return captured.str();
        }
    };
}

class IntegrationTestFixture : public testing::Test {
public:
    CodeFabFacade facade;
};

TEST_F(IntegrationTestFixture, ClassInheritanceArrayFieldRecursionAndClosuresWorkTogetherThroughRealFacade) {
    // 재귀 함수(fact) + 배열 필드를 갖는 클래스 + 상속/Super + 클로저/정적 바인딩 +
    // 리터럴 상수 폴딩을 한 프로그램 안에서 함께 사용한다.
    facade.execute(
        "Func fact(n) {"
        "  if (n <= 1) return 1;"
        "  return n * fact(n - 1);"
        "}"
        "Class Storage {"
        "  init(size) { this.items = Array(size); }"
        "  set(i, v) { this.items[i] = v; }"
        "  get(i) { return this.items[i]; }"
        "}"
        "Class LoggingStorage : Storage {"
        "  set(i, v) { Super.set(i, v); print \"stored\"; }"
        "}"
        "var s = LoggingStorage(2);"
        "s.set(0, fact(5));"
        "s.set(1, 1 + 2 * 3 - (4 / 2));");

    EXPECT_EQ(captureStdout(facade, "print s.get(0);"), "120\n");
    EXPECT_EQ(captureStdout(facade, "print s.get(1);"), "5\n");

    // 클로저가 선언 시점의 어휘적 스코프(바깥의 전역 counter)에 고정되는지도
    // 같은 통합 시나리오 안에서 확인한다 - Chapter 5 정적 바인딩이 실제로
    // 클래스/함수/배열과 뒤섞여도 여전히 올바르게 동작해야 한다.
    EXPECT_EQ(
        captureStdout(
            facade,
            "var counter = 0;"
            "{"
            "  Func increment() { return counter + 1; }"
            "  print increment();"
            "  var counter = 100;"
            "  print increment();"
            "}"),
        "1\n1\n");
}

TEST_F(IntegrationTestFixture, InheritedMethodCanUseImportedFunctionResultAndClassIsUsableThroughAlias) {
    TestPipeline pipeline;
    pipeline.file_system.addFile(
        "shapes.txt",
        "Class Rectangle {"
        "  init(w, h) { this.w = w; this.h = h; }"
        "  area() { return this.w * this.h; }"
        "}"
        "Func makeSquare(side) { return Rectangle(side, side); }");

    string output = pipeline.run(
        "import \"shapes.txt\" alias shapes;"
        "var sq = shapes.makeSquare(4);"
        "print sq.area();"
        "var r = shapes.Rectangle(2, 3);"
        "print r.area();");

    EXPECT_EQ(output, "16\n6\n");
}

TEST_F(IntegrationTestFixture, ImportedClassInheritedByLocalSubclassOverridesMethodAndCallsSuper) {
    // import로 들여온 클래스를 이 스크립트에서 상속해 오버라이딩하고, Super로
    // 부모(=import된 클래스)의 메서드를 호출한다 - import와 상속/Super가 함께
    // 동작하는 것을 확인한다. `Class X : ...`는 `.`이 없는 단일 식별자만
    // 허용하므로, base.Robot을 먼저 지역 변수에 담아 그 이름으로 상속한다.
    TestPipeline pipeline;
    pipeline.file_system.addFile(
        "robot.txt",
        "Class Robot {"
        "  move(dist) { print \"move\"; }"
        "}");

    string output = pipeline.run(
        "import \"robot.txt\" alias base;"
        "var Robot = base.Robot;"
        "Class SpeedRobot : Robot {"
        "  move(dist) { Super.move(dist); print \"Speeeed!\"; }"
        "}"
        "SpeedRobot().move(3);");

    EXPECT_EQ(output, "move\nSpeeeed!\n");
}

#endif
