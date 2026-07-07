#pragma once

#include "AssemblerUnit/Parser/Expression.h"
#include "AssemblerUnit/Parser/Statement.h"

#define interface struct

interface Visitor {
public:
	virtual ~Visitor() = default;
	virtual void visit(const Expression* expr) = 0;
	virtual void visit(const Statement* stmt) = 0;
};