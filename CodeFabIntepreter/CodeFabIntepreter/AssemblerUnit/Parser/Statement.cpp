#include "Statement.h"
#include "../../Visitor.h"

void VarDeclareStmt::accept(Visitor& v) {
	v.visit(this);
}
