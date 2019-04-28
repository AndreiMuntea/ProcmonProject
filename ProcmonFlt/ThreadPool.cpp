#include <fltKernel.h>

#include "ThreadPool.hpp"
#include "trace.hpp"
#include "ThreadPool.tmh"

#include <cpp_lock.hpp>
#include <cpp_lockguard.hpp>


#define TERMINATION_SIGNALED_EVENT 0
#define ITEM_ENQUEUED_NOTIFICATION_EVENT 1 

Cpp::ThreadPoolWorkItem::ThreadPoolWorkItem(
    _In_ PFUNC_ThreadPoolCallback Callback,
    _In_ PFUNC_ThreadPoolCleanupCallback CleanupCallback,
    _In_opt_ PVOID Context
) : callback{Callback},
    cleanupCallback{CleanupCallback},
    context{Context}
{
    Validate();
}

Cpp::ThreadPool::ThreadPool(_In_ UINT8 NoThreads)
{
    if (NoThreads > 15 || NoThreads == 0)
    {
        NoThreads = 15;
    }

    OBJECT_ATTRIBUTES attributes = {};
    InitializeObjectAttributes(&attributes, nullptr, OBJ_KERNEL_HANDLE, nullptr, nullptr);

    KeInitializeEvent(&this->events[0], NotificationEvent, false);
    KeInitializeEvent(&this->events[1], NotificationEvent, false);

    threads = (HANDLE*)ExAllocatePoolWithTag(NonPagedPool, NoThreads * sizeof(HANDLE), DRV_TAG_TPT);
    if (!threads)
    {
        MyDriverLogError("Allocating new threads failed!");
        NT_ASSERT(false);
        return;
    }

    noThreads = NoThreads;
    RtlZeroMemory(threads, NoThreads * sizeof(HANDLE));

    for (UINT8 i = 0; i < noThreads; ++i)
    {
        auto status = PsCreateSystemThread(&threads[i], GENERIC_ALL | SYNCHRONIZE, &attributes, nullptr, nullptr, ThreadPool::StartRoutine, this);
        if (!NT_VERIFY(NT_SUCCESS(status)))
        {
            MyDriverLogError("PsCreateSystemThread failed with status 0x%X", status);
            Shutdown();
            CleanupThreadPool();
            return;
        }
    }

    Validate();
}

Cpp::ThreadPool::~ThreadPool()
{
    Shutdown();
    CleanupThreadPool();
}

void Cpp::ThreadPool::Shutdown()
{
    pendingShutdown = true;
    KeSetEvent(&this->events[TERMINATION_SIGNALED_EVENT], 0, false);
}

NTSTATUS Cpp::ThreadPool::EnqueueItem(
    _In_ PFUNC_ThreadPoolCallback Callback,
    _In_ PFUNC_ThreadPoolCleanupCallback CleanupCallback,
    _In_opt_ PVOID Context
)
{
    auto element = new ThreadPoolWorkItem(Callback, CleanupCallback, Context);
    if (!element)
    {
        MyDriverLogError("Not enough resources");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ExclusiveLockguard guard(&lock);
    if (pendingShutdown)
    {
        delete element;
        return STATUS_TOO_LATE;
    }

    workItems.Push(element);
    if (workItems.GetNoElements() == 1)
    {
        KeSetEvent(&events[ITEM_ENQUEUED_NOTIFICATION_EVENT], 0, false);
    }
}

void Cpp::ThreadPool::WaitThreads()
{
    if (!threads)
    {
        return;
    }

    for (UINT8 i = 0; i < noThreads; ++i)
    {
        if (threads[i])
        {
            ZwWaitForSingleObject(threads[i], false, nullptr);
            ZwClose(threads[i]);
            threads[i] = nullptr;
        }
    }

    ExFreePoolWithTag(threads, DRV_TAG_TPT);
    threads = nullptr;
}

void Cpp::ThreadPool::CleanupThreadPool()
{
    WaitThreads();
}

Cpp::ThreadPoolWorkItem* Cpp::ThreadPool::GetWorkerItem()
{
    ExclusiveLockguard guard(&lock);
    ThreadPoolWorkItem* result = nullptr;

    if (workItems.IsEmpty())
    {
        return nullptr;
    }

    result = workItems.Pop();
    if (workItems.IsEmpty())
    {
        KeClearEvent(&events[ITEM_ENQUEUED_NOTIFICATION_EVENT]);
    }

    return result;
}

_Use_decl_annotations_
void Cpp::ThreadPool::StartRoutine(
    _In_ PVOID Context
)
{
    auto threadPool = reinterpret_cast<ThreadPool*>(Context);
    PVOID waitEvents[2] = { &threadPool->events[0], &threadPool->events[1] };

    MyDriverLogInfo("We are into StartRoutine");

    while (!threadPool->pendingShutdown)
    {
        auto item = threadPool->GetWorkerItem();
        if (item)
        {
            item->callback(item->context);
            delete item;
            continue;
        }

        KeWaitForMultipleObjects(2, waitEvents, WaitAny, Executive, KernelMode, false, nullptr, nullptr);
    }

    auto item = threadPool->GetWorkerItem();
    while (item)
    {
        item->cleanupCallback(item->context);
        delete item;
        item = threadPool->GetWorkerItem();
    }

    MyDriverLogInfo("Exiting StartRoutine");
}
