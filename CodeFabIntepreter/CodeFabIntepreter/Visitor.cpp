#include "Visitor.h"

void LiteralExpr::accept(ExprVisitor& visitor) const { visitor.visitLiteralExpr(*this); }
void VariableExpr::accept(ExprVisitor& visitor) const { visitor.visitVariableExpr(*this); }
void AssignExpr::accept(ExprVisitor& visitor) const { visitor.visitAssignExpr(*this); }
void BinaryExpr::accept(ExprVisitor& visitor) const { visitor.visitBinaryExpr(*this); }
void UnaryExpr::accept(ExprVisitor& visitor) const { visitor.visitUnaryExpr(*this); }
void GroupingExpr::accept(ExprVisitor& visitor) const { visitor.visitGroupingExpr(*this); }
void LogicalExpr::accept(ExprVisitor& visitor) const { visitor.visitLogicalExpr(*this); }
void ArrayExpr::accept(ExprVisitor& visitor) const { visitor.visitArrayExpr(*this); }
void IndexExpr::accept(ExprVisitor& visitor) const { visitor.visitIndexExpr(*this); }
void IndexSetExpr::accept(ExprVisitor& visitor) const { visitor.visitIndexSetExpr(*this); }
void CallExpr::accept(ExprVisitor& visitor) const { visitor.visitCallExpr(*this); }

void ExpressionStmt::accept(StmtVisitor& visitor) { visitor.visitExpressionStmt(*this); }
void IfStmt::accept(StmtVisitor& visitor) { visitor.visitIfStmt(*this); }
void BlockStmt::accept(StmtVisitor& visitor) { visitor.visitBlockStmt(*this); }
void VarDeclareStmt::accept(StmtVisitor& visitor) { visitor.visitVarDeclareStmt(*this); }
void PrintStmt::accept(StmtVisitor& visitor) { visitor.visitPrintStmt(*this); }
void ForStmt::accept(StmtVisitor& visitor) { visitor.visitForStmt(*this); }
void FunctionDeclStmt::accept(StmtVisitor& visitor) { visitor.visitFunctionDeclStmt(*this); }
void ReturnStmt::accept(StmtVisitor& visitor) { visitor.visitReturnStmt(*this); }
