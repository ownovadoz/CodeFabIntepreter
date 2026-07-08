#include "AssemblerUnit.h"

#include "Parser/Parser.h"
#include "Tokenizer/Lexer.h"

#include <memory>
#include <vector>

using std::unique_ptr;
using std::vector;

Statement* AssemblerUnit::assemble(const string& code_line) {
	Lexer lexer(code_line);
	vector<Token> tokens = lexer.scanTokens();

	Parser parser;
	vector<unique_ptr<Statement>> program = parser.parse(tokens);
	if (program.empty()) return nullptr;

	return program[0].release();
}
