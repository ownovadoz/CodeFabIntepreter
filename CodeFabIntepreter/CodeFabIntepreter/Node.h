#pragma once

class Visitor;

#define interface struct

interface Node {
	virtual void accept(Visitor& v) = 0;
};