#pragma once

#include "Visitor.h"

using std::vector;

#define interface struct

interface Node {
	virtual void accept(Visitor& v) = 0;
};