#include "CodeFabException.h"

#include <gmock/gmock.h>

TEST(CodeFabExceptionTest, LineConstructorFormatsMessageWithLineNumber) {
	CodeFabException error(7, "unexpected character");

	EXPECT_EQ(error.getLine(), 7);
	EXPECT_EQ(error.getMessage(), "unexpected character");
	EXPECT_STREQ(error.what(), "[line 7] Error: unexpected character");
}