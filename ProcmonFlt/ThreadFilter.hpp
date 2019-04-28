#ifndef __THREAD_FILTER_HPP__ 
#define __THREAD_FILTER_HPP__

#include "cpp_allocator_object.hpp"

namespace Minifilter
{
    class ThreadFilter : public Cpp::CppNonPagedObject<'TFT#'>
    {
    public:
        ThreadFilter();
        virtual ~ThreadFilter();

        static void
        CreateThreadNotifyRoutine(
            _In_ HANDLE ProcessId,
            _In_ HANDLE ThreadId,
            _In_ BOOLEAN Create
        );
    };
}

#endif //__THREAD_FILTER_HPP__