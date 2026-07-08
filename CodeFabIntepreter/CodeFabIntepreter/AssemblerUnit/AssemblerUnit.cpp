#include "AssemblerUnit.h"

#include "Parser/Parser.h"
#include "Tokenizer/Lexer.h"

#include <vector>

using std::vector;

unique_ptr<Statement> AssemblerUnit::assemble(const string& code_line) {
	Lexer lexer(code_line);
	vector<Token> tokens = lexer.scanTokens();

	Parser parser;
	return parser.parse(tokens);
}
