#include "ProcessUtils.hpp"

#define SYSTEM_PID ((HANDLE)4)
#define IDLE_PID ((HANDLE)0)

bool Minifilter::ProcessUtils::IsSystemOrIdleProcess(const HANDLE ProcessId)
{
    return ProcessId == SYSTEM_PID || ProcessId == IDLE_PID;
}
