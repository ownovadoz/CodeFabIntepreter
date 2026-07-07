#pragma once

#include "../../Node.h"
#include "../Tokenize/Token.h"

#include <vector>

using std::vector;

class Parser
{
public:
	Node* parse(vector<Token> tokens);

private:

};