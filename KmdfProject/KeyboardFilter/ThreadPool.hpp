#ifndef __THREAD_POOL_HPP__ 
#define __THREAD_POOL_HPP__

#include <cpp_lock.hpp>
#include "Queue.hpp"

#define DRV_TAG_TPT 'TPT#'   // #TPT - ThreadPool Tag

namespace Cpp
{
    typedef void FUNC_ThreadPoolCallback(_In_opt_ PVOID Context);
    typedef FUNC_ThreadPoolCallback *PFUNC_ThreadPoolCallback;

    typedef void FUNC_ThreadPoolCleanupCallback(_In_opt_ PVOID Context);
    typedef FUNC_ThreadPoolCleanupCallback *PFUNC_ThreadPoolCleanupCallback;

    class ThreadPoolWorkItem : public LinkedListEntry
    {
        friend class ThreadPool;
    public:
        ThreadPoolWorkItem(
            _In_ PFUNC_ThreadPoolCallback Callback,
            _In_ PFUNC_ThreadPoolCleanupCallback CleanupCallback,
            _In_opt_ PVOID Context
        );

        virtual ~ThreadPoolWorkItem() = default;

    private:
        PFUNC_ThreadPoolCallback callback = nullptr;
        PFUNC_ThreadPoolCleanupCallback cleanupCallback = nullptr;
        PVOID context = nullptr;
    };


    class ThreadPool : public Cpp::CppNonPagedObject<DRV_TAG_TPT>
    {
    public:
        ThreadPool(_In_ UINT8 NoThreads);
        virtual ~ThreadPool();

        void Shutdown();

        NTSTATUS EnqueueItem(
            _In_ PFUNC_ThreadPoolCallback Callback,
            _In_ PFUNC_ThreadPoolCleanupCallback CleanupCallback,
            _In_opt_ PVOID Context
        );

    private:
        Queue<ThreadPoolWorkItem> workItems;
        KEVENT events[2] = { {},{} };
        bool pendingShutdown = false;
        UINT8 noThreads = 0;
        HANDLE* threads = nullptr;
        Cpp::Spinlock lock;

        void WaitThreads();
        void CleanupThreadPool();

        ThreadPoolWorkItem* GetWorkerItem();

        _Function_class_(KSTART_ROUTINE)
            static void StartRoutine(_In_ PVOID Context);
    };
}

#endif //__THREAD_POOL_HPP__