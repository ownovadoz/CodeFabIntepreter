#include "AssemblerUnit.h"

#include <gmock/gmock.h>

TEST(AssemblerUnitTest, EmptyInputReturnsNullptr) {
	AssemblerUnit assembler_unit;

	EXPECT_EQ(assembler_unit.assemble(""), nullptr);
}

TEST(AssemblerUnitTest, ParserExceptionPropagatesToCaller) {
	AssemblerUnit assembler_unit;

	EXPECT_THROW(assembler_unit.assemble("="), std::exception);
}
