#include <gmock/gmock.h>
#include "PromptShell.h"

int main() {
#ifdef _DEBUG
	::testing::InitGoogleMock();
	return RUN_ALL_TESTS();
#else
	PromptShell shell;
	shell.runPrompt();
#endif
}