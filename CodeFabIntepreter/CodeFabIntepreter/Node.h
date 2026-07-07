#pragma once

#include <vector>

using std::vector;

#define interface struct

interface Node {

	virtual void accept() = 0;
};