#ifndef __PROCESS_UTILS_HPP__ 
#define __PROCESS_UTILS_HPP__

#include <fltKernel.h>

namespace Minifilter
{
    class ProcessUtils
    {
    public:
        static bool IsSystemOrIdleProcess(const HANDLE ProcessId);
    };
}

#endif //__PROCESS_UTILS_HPP__