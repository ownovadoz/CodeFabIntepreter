#pragma once

#include "AssemblerUnit/Parser/Expression.h"
#include "AssemblerUnit/Parser/Statement.h"

class ExprVisitor {
public:
	virtual ~ExprVisitor() = default;
	virtual void visitLiteralExpr(const LiteralExpr& expr) = 0;
	virtual void visitVariableExpr(const VariableExpr& expr) = 0;
	virtual void visitAssignExpr(const AssignExpr& expr) = 0;
	virtual void visitBinaryExpr(const BinaryExpr& expr) = 0;
	virtual void visitUnaryExpr(const UnaryExpr& expr) = 0;
	virtual void visitGroupingExpr(const GroupingExpr& expr) = 0;
	virtual void visitLogicalExpr(const LogicalExpr& expr) = 0;
	virtual void visitArrayExpr(const ArrayExpr& expr) = 0;
	virtual void visitIndexExpr(const IndexExpr& expr) = 0;
	virtual void visitIndexSetExpr(const IndexSetExpr& expr) = 0;
	virtual void visitCallExpr(const CallExpr& expr) = 0;
};

class StmtVisitor {
public:
	virtual ~StmtVisitor() = default;
	virtual void visitExpressionStmt(ExpressionStmt& stmt) = 0;
	virtual void visitIfStmt(IfStmt& stmt) = 0;
	virtual void visitBlockStmt(BlockStmt& stmt) = 0;
	virtual void visitVarDeclareStmt(VarDeclareStmt& stmt) = 0;
	virtual void visitPrintStmt(PrintStmt& stmt) = 0;
	virtual void visitForStmt(ForStmt& stmt) = 0;
	virtual void visitFunctionDeclStmt(FunctionDeclStmt& stmt) = 0;
	virtual void visitReturnStmt(ReturnStmt& stmt) = 0;
};
