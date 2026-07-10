#include "FileModeShell.h"

void FileModeShell::beforeExecuteLine(int, const string& line_text) {
	code_line = line_text;
}
