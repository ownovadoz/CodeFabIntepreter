#include "DebugCommand.h"
#include "DebugModeShell.h"

bool StepCommand::execute(DebugModeShell& shell, istringstream&) const {
	shell.enterStepMode();
	return true;
}

bool NextCommand::execute(DebugModeShell& shell, istringstream&) const {
	shell.enterNextMode();
	return true;
}

bool ContinueCommand::execute(DebugModeShell& shell, istringstream&) const {
	shell.enterContinueMode();
	return true;
}

bool BreakCommand::execute(DebugModeShell& shell, istringstream& args) const {
	int line;
	if (args >> line) shell.addBreakpoint(line);
	return false;
}

bool RemoveBreakpointCommand::execute(DebugModeShell& shell, istringstream& args) const {
	int line;
	if (args >> line) shell.removeBreakpoint(line);
	return false;
}

bool BreakpointsCommand::execute(DebugModeShell& shell, istringstream&) const {
	shell.printBreakpoints();
	return false;
}

bool WatchCommand::execute(DebugModeShell& shell, istringstream& args) const {
	string name;
	if (args >> name) shell.addWatch(name);
	return false;
}

bool UnwatchCommand::execute(DebugModeShell& shell, istringstream& args) const {
	string name;
	if (args >> name) shell.removeWatch(name);
	return false;
}

bool WatchesCommand::execute(DebugModeShell& shell, istringstream&) const {
	shell.printWatchedVariables();
	return false;
}

bool InspectCommand::execute(DebugModeShell& shell, istringstream&) const {
	shell.printInspect();
	return false;
}
