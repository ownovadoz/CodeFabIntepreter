#pragma once
#include <sstream>

using std::istringstream;

class DebugModeShell;

// 디버그 콘솔에서 입력받은 한 명령어를 표현하는 Command 패턴의 인터페이스.
// execute가 true를 반환하면 정지 상태를 풀고 실행을 재개하고, false를 반환하면
// 계속 멈춰서 다음 명령어를 기다린다.
class DebugCommand {
public:
	virtual ~DebugCommand() = default;
	virtual bool execute(DebugModeShell& shell, istringstream& args) const = 0;
};

class StepCommand : public DebugCommand {
public:
	bool execute(DebugModeShell& shell, istringstream& args) const override;
};

class NextCommand : public DebugCommand {
public:
	bool execute(DebugModeShell& shell, istringstream& args) const override;
};

class ContinueCommand : public DebugCommand {
public:
	bool execute(DebugModeShell& shell, istringstream& args) const override;
};

class BreakCommand : public DebugCommand {
public:
	bool execute(DebugModeShell& shell, istringstream& args) const override;
};

class RemoveBreakpointCommand : public DebugCommand {
public:
	bool execute(DebugModeShell& shell, istringstream& args) const override;
};

class BreakpointsCommand : public DebugCommand {
public:
	bool execute(DebugModeShell& shell, istringstream& args) const override;
};

class WatchCommand : public DebugCommand {
public:
	bool execute(DebugModeShell& shell, istringstream& args) const override;
};

class UnwatchCommand : public DebugCommand {
public:
	bool execute(DebugModeShell& shell, istringstream& args) const override;
};

class WatchesCommand : public DebugCommand {
public:
	bool execute(DebugModeShell& shell, istringstream& args) const override;
};

class InspectCommand : public DebugCommand {
public:
	bool execute(DebugModeShell& shell, istringstream& args) const override;
};
