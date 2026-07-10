#ifdef _DEBUG

#include "Interpreter.h"

#include "../AssemblerUnit/AssemblerUnit.h"
#include "../CodeFabException.h"

#include <gmock/gmock.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

using std::ofstream;
using std::ostringstream;
using std::string;
using std::unordered_map;

namespace {
    // "import \"path\" alias x;"가 읽어들일 파일 내용을 흉내 내는 가짜 파일 시스템.
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

    string captureStdout(Interpreter& interpreter, const vector<unique_ptr<Statement>>& statements) {
        ostringstream captured;
        std::streambuf* original_buf = std::cout.rdbuf(captured.rdbuf());
        interpreter.interpret(statements);
        std::cout.rdbuf(original_buf);
        return captured.str();
    }
}

class ImportTestFixture : public testing::Test {
public:
    FakeFileSystem file_system;
    Interpreter interpreter{
        [this](const string& path) { return file_system.exists(path); },
        [this](const string& path) { return file_system.read(path); }
    };
    AssemblerUnit assembler;
};

TEST_F(ImportTestFixture, ImportBindsImportedFunctionUnderAliasAndCanBeCalled) {
    file_system.addFile("greet.txt", "Func greet() { return \"hi\"; }");

    auto statements = assembler.assemble("import \"greet.txt\" alias g; print g.greet();");

    EXPECT_EQ(captureStdout(interpreter, statements), "hi\n");
}

TEST_F(ImportTestFixture, ImportBindsImportedVariableUnderAlias) {
    file_system.addFile("consts.txt", "var x = 42;");

    auto statements = assembler.assemble("import \"consts.txt\" alias c; print c.x;");

    EXPECT_EQ(captureStdout(interpreter, statements), "42\n");
}

TEST_F(ImportTestFixture, ImportedFileTopLevelVariablesDoNotLeakIntoImportingScope) {
    file_system.addFile("m.txt", "var secret = 1;");

    auto statements = assembler.assemble("import \"m.txt\" alias m;");

    interpreter.interpret(statements);

    EXPECT_THROW(interpreter.getVariableValue("secret"), CodeFabException);
}

TEST_F(ImportTestFixture, ImportingMissingFileThrowsCodeFabException) {
    auto statements = assembler.assemble("import \"missing.txt\" alias m;");

    try {
        interpreter.interpret(statements);
        FAIL() << "CodeFabException을 기대했지만 던져지지 않았습니다.";
    }
    catch (const CodeFabException& exception) {
        EXPECT_THAT(exception.what(), ::testing::HasSubstr("파일을 찾을 수 없습니다"));
    }
}

TEST_F(ImportTestFixture, CircularImportThrowsCodeFabException) {
    file_system.addFile("a.txt", "import \"b.txt\" alias b;");
    file_system.addFile("b.txt", "import \"a.txt\" alias a;");

    auto statements = assembler.assemble("import \"a.txt\" alias a;");

    try {
        interpreter.interpret(statements);
        FAIL() << "CodeFabException을 기대했지만 던져지지 않았습니다.";
    }
    catch (const CodeFabException& exception) {
        EXPECT_THAT(exception.what(), ::testing::HasSubstr("순환 import"));
    }
}

TEST_F(ImportTestFixture, StaticErrorInImportedFilePropagatesAsCodeFabException) {
    // var a = a + 1;은 Checker가 초기화식 자기참조로 거부하는 입력이다.
    file_system.addFile("bad.txt", "var a = a + 1;");

    auto statements = assembler.assemble("import \"bad.txt\" alias b;");

    EXPECT_THROW(interpreter.interpret(statements), CodeFabException);
}

TEST_F(ImportTestFixture, RuntimeErrorInImportedFileDuringInterpretRestoresOuterScopeAndPropagates) {
    // 정적 검사는 통과하지만 실행 중(0으로 나누기)에 실패하는 경우로,
    // executeImportStmt가 재귀 interpret() 도중 던져진 예외를 잡아 globals/
    // environment를 되돌리고 다시 던지는 catch(...) 경로를 검증한다.
    file_system.addFile("bad.txt", "print 1 / 0;");

    auto import_statements = assembler.assemble("import \"bad.txt\" alias b;");
    EXPECT_THROW(interpreter.interpret(import_statements), CodeFabException);

    // import 실패 이후에도 바깥 스코프(globals/environment)가 정상 복구되어
    // 계속 사용할 수 있어야 한다.
    auto followup_statements = assembler.assemble("var x = 1; print x;");
    EXPECT_EQ(captureStdout(interpreter, followup_statements), "1\n");
}

TEST_F(ImportTestFixture, SameFileImportedTwiceWithDifferentAliasesIsNotTreatedAsCircular) {
    file_system.addFile("shared.txt", "var x = 1;");

    auto statements = assembler.assemble(
        "import \"shared.txt\" alias first; import \"shared.txt\" alias second; print first.x; print second.x;");

    EXPECT_EQ(captureStdout(interpreter, statements), "1\n1\n");
}

TEST_F(ImportTestFixture, AccessingUndefinedMemberOnImportedNamespaceThrowsCodeFabException) {
    file_system.addFile("m.txt", "var x = 1;");

    auto statements = assembler.assemble("import \"m.txt\" alias m; print m.missing;");

    EXPECT_THROW(interpreter.interpret(statements), CodeFabException);
}

TEST_F(ImportTestFixture, CallingImportedNamespaceDirectlyThrowsCodeFabException) {
    file_system.addFile("m.txt", "var x = 1;");

    auto statements = assembler.assemble("import \"m.txt\" alias m; m();");

    EXPECT_THROW(interpreter.interpret(statements), CodeFabException);
}

TEST(ImportRealFileSystemTest, ImportOfRealFileOnDiskReadsAndExecutesItsActualContent) {
    // Interpreter()를 기본 인자(실제 파일 시스템)로 생성해, 가짜 파일 시스템을
    // 주입하지 않는 기본 defaultFileExists/defaultReadSource 경로 자체를 검증한다.
    string path = (std::filesystem::temp_directory_path() / "codefab_import_test.txt").string();
    {
        ofstream file(path, std::ios::binary);
        file << "Func greet() { return \"hi\"; }";
    }

    Interpreter interpreter;
    AssemblerUnit assembler;
    auto statements = assembler.assemble("import \"" + path + "\" alias m; print m.greet();");

    ostringstream captured;
    std::streambuf* original_buf = std::cout.rdbuf(captured.rdbuf());
    interpreter.interpret(statements);
    std::cout.rdbuf(original_buf);

    std::filesystem::remove(path);

    EXPECT_EQ(captured.str(), "hi\n");
}

#endif
