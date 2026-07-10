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
	virtual void visitCallExpr(const CallExpr& expr) = 0;
	virtual void visitGetExpr(const GetExpr& expr) = 0;
	virtual void visitSetExpr(const SetExpr& expr) = 0;
	virtual void visitThisExpr(const ThisExpr& expr) = 0;
	virtual void visitSuperExpr(const SuperExpr& expr) = 0;
	virtual void visitInstanceOfExpr(const InstanceOfExpr& expr) = 0;
	virtual void visitArrayExpr(const ArrayExpr& expr) = 0;
	virtual void visitIndexExpr(const IndexExpr& expr) = 0;
	virtual void visitIndexSetExpr(const IndexSetExpr& expr) = 0;
};

class StmtVisitor {
public:
	virtual ~StmtVisitor() = default;
	virtual void visitExpressionStmt(const ExpressionStmt& stmt) = 0;
	virtual void visitIfStmt(const IfStmt& stmt) = 0;
	virtual void visitBlockStmt(const BlockStmt& stmt) = 0;
	virtual void visitVarDeclareStmt(const VarDeclareStmt& stmt) = 0;
	virtual void visitPrintStmt(const PrintStmt& stmt) = 0;
	virtual void visitForStmt(const ForStmt& stmt) = 0;
	virtual void visitFunctionStmt(const FunctionStmt& stmt) = 0;
	virtual void visitReturnStmt(const ReturnStmt& stmt) = 0;
	virtual void visitClassStmt(const ClassStmt& stmt) = 0;
	virtual void visitImportStmt(const ImportStmt& stmt) = 0;
};
