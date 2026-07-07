#pragma once

#include "../../Node.h"
#include "../Tokenizer/Token.h"

#include <vector>

using std::vector;

class Parser
{
public:
	Node* parse(const vector<Token>& tokens);
};