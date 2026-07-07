#pragma once

#include <vector>

using std::vector;

#define interface struct

interface Node {

	virtual void accept() = 0;
};

class Statement : public Node {
public:
	void accept() override;

private:
	vector<Node*> children;
};

class ExpressionOrToken : public Node
{
public:
	void accept() override;
};

class Expression : public ExpressionOrToken
{
public:
	void accept() override;

private:
	vector<ExpressionOrToken*> children;
};

class Token : public ExpressionOrToken
{
public:
	void accept() override;
};