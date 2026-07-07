#include "Expression.h"
#include "../../Visitor.h"

void LiteralExpr::accept(Visitor& v) {
	v.visit(this);
}

void BinaryExpr::accept(Visitor& v) {
	v.visit(this);
}
