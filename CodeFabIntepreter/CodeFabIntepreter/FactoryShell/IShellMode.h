#pragma once

class IShellMode {
public:
	virtual ~IShellMode() = default;
	virtual void enter() = 0;
};
