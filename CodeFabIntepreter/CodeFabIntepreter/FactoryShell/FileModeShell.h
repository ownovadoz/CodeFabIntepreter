#pragma once
#include "FileBackedShell.h"

#include <string>

using std::string;

class FileModeShell : public FileBackedShell {
public:
	using FileBackedShell::FileBackedShell;

	const string& getLastLine() const { return code_line; }

protected:
	void beforeExecuteLine(int line_number, const string& line_text) override;

private:
	string code_line;
};
