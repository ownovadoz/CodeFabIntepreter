#include "AssemblerUnit.h"

#include "Parser/Statement.h"

#include <gmock/gmock.h>

TEST(AssemblerUnitTest, EmptyInputReturnsNoStatements) {
	AssemblerUnit assembler_unit;

	EXPECT_TRUE(assembler_unit.assemble("").empty());
}

TEST(AssemblerUnitTest, ParserExceptionPropagatesToCaller) {
	AssemblerUnit assembler_unit;

	EXPECT_THROW(assembler_unit.assemble("="), std::exception);
}

TEST(AssemblerUnitTest, MultipleStatementsOnOneLineAreAllReturnedInOrder) {
	// 세미콜론으로 구분된 여러 문장이 한 줄에 있으면, 첫 문장만이 아니라
	// 전부가 순서대로 반환되어야 한다.
	AssemblerUnit assembler_unit;

	auto statements = assembler_unit.assemble("var a = 3; var b = 4;");

	ASSERT_EQ(statements.size(), 2u);
	EXPECT_NE(dynamic_cast<VarDeclareStmt*>(statements[0].get()), nullptr);
	EXPECT_NE(dynamic_cast<VarDeclareStmt*>(statements[1].get()), nullptr);
}
