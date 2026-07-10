#include "Visitor.h"

void LiteralExpr::accept(ExprVisitor& visitor) const { visitor.visitLiteralExpr(*this); }
void VariableExpr::accept(ExprVisitor& visitor) const { visitor.visitVariableExpr(*this); }
void AssignExpr::accept(ExprVisitor& visitor) const { visitor.visitAssignExpr(*this); }
void BinaryExpr::accept(ExprVisitor& visitor) const { visitor.visitBinaryExpr(*this); }
void UnaryExpr::accept(ExprVisitor& visitor) const { visitor.visitUnaryExpr(*this); }
void GroupingExpr::accept(ExprVisitor& visitor) const { visitor.visitGroupingExpr(*this); }
void LogicalExpr::accept(ExprVisitor& visitor) const { visitor.visitLogicalExpr(*this); }
void CallExpr::accept(ExprVisitor& visitor) const { visitor.visitCallExpr(*this); }
void GetExpr::accept(ExprVisitor& visitor) const { visitor.visitGetExpr(*this); }
void SetExpr::accept(ExprVisitor& visitor) const { visitor.visitSetExpr(*this); }
void ThisExpr::accept(ExprVisitor& visitor) const { visitor.visitThisExpr(*this); }
void SuperExpr::accept(ExprVisitor& visitor) const { visitor.visitSuperExpr(*this); }
void InstanceOfExpr::accept(ExprVisitor& visitor) const { visitor.visitInstanceOfExpr(*this); }

void ExpressionStmt::accept(StmtVisitor& visitor) const { visitor.visitExpressionStmt(*this); }
void IfStmt::accept(StmtVisitor& visitor) const { visitor.visitIfStmt(*this); }
void BlockStmt::accept(StmtVisitor& visitor) const { visitor.visitBlockStmt(*this); }
void VarDeclareStmt::accept(StmtVisitor& visitor) const { visitor.visitVarDeclareStmt(*this); }
void PrintStmt::accept(StmtVisitor& visitor) const { visitor.visitPrintStmt(*this); }
void ForStmt::accept(StmtVisitor& visitor) const { visitor.visitForStmt(*this); }
void FunctionStmt::accept(StmtVisitor& visitor) const { visitor.visitFunctionStmt(*this); }
void ReturnStmt::accept(StmtVisitor& visitor) const { visitor.visitReturnStmt(*this); }
void ClassStmt::accept(StmtVisitor& visitor) const { visitor.visitClassStmt(*this); }
