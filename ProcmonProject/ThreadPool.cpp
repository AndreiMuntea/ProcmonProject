#include "ThreadPool.hpp"
#include "trace.h"
#include "ThreadPool.tmh"

#define TERMINATION_SIGNALED_EVENT 0
#define ITEM_ENQUEUED_NOTIFICATION_EVENT 1  

ThreadWorkerItem::ThreadWorkerItem(
    _In_opt_ std::shared_ptr<ThreadContext> Context,
    _In_ std::function<void(std::shared_ptr<ThreadContext>)> Callback
) : context { Context },
    callback { Callback }
{}

ThreadPool::ThreadPool(
    _In_ uint8_t  NoThreads
)
{
    if (NoThreads == 0 || NoThreads > std::thread::hardware_concurrency())
    {
        ConsoleAppLogError("Invalid number of threads passed <%d>. Will use default value %d", NoThreads, static_cast<int>(std::thread::hardware_concurrency()));
        NoThreads = static_cast<uint8_t>(std::thread::hardware_concurrency());
    }    
    
    events.resize(2, nullptr);
    threads.resize(NoThreads, nullptr);

    for (size_t i = 0; i < events.size(); ++i)
    {
        events[i] = CreateEvent(nullptr, true, false, nullptr);
        if (!events[i])
        {
            CleanupThreadPool();
            ConsoleAppLogError("CreateEvent has failed with GLE=%d", GetLastError());
            throw std::exception("CreateEvent has failed");
        }
    }

    for (size_t i = 0; i < threads.size(); ++i)
    {
        threads[i] = CreateThread(nullptr, 0, &ThreadPool::DoWork, this, 0, 0);
        if (!threads[i])
        {
            Shutdown();
            CleanupThreadPool();
            ConsoleAppLogError("CreateThread has failed with GLE=%d", GetLastError());
            throw std::exception("CreateThread has failed");
        }
    }
    ConsoleAppLogInfo("All threads were successfully initialized");
}

ThreadPool::~ThreadPool()
{
    Shutdown();
    CleanupThreadPool();
}

void 
ThreadPool::Shutdown()
{
    pendingShutdown = true;
    SetEvent(events[TERMINATION_SIGNALED_EVENT]);
    WaitThreads();
}

void 
ThreadPool::EnqueueItem(
    _In_opt_ std::shared_ptr<ThreadContext> Context,
    _In_ std::function<void(std::shared_ptr<ThreadContext>)> Callback
)
{
    std::lock_guard<std::mutex> guard(itemsLock);

    if (pendingShutdown)
    {
        ConsoleAppLogError("Thread pool is pending shutdown!");
        throw std::exception("Thread pool is pending shutdown!");
    }

    items.emplace(std::make_shared<ThreadWorkerItem>(Context, Callback));
    if (items.size() == 1)
    {
        SetEvent(events[ITEM_ENQUEUED_NOTIFICATION_EVENT]);
    }
}

size_t 
ThreadPool::GetNoThreads() const
{
    return threads.size();
}

void 
ThreadPool::WaitThreads()
{
    for (size_t i = 0; i < threads.size(); ++i)
    {
        if (threads[i])
        {
            WaitForSingleObject(threads[i], INFINITE);
            CloseHandle(threads[i]);
            threads[i] = nullptr;
        }
    }
}

void 
ThreadPool::DisposeEvents()
{
    for (size_t i = 0; i < events.size(); ++i)
    {
        if (events[i])
        {
            CloseHandle(events[i]);
            events[i] = nullptr;
        }
    }
}

void
ThreadPool::CleanupThreadPool()
{
    WaitThreads();
    DisposeEvents();
}

std::shared_ptr<ThreadWorkerItem> 
ThreadPool::GetWorkerItem()
{
    std::lock_guard<std::mutex> guard{ itemsLock };

    if (items.empty())
    {
        return nullptr;
    }

    auto workerItem = items.front(); 
    items.pop();

    if (items.empty())
    {
        ResetEvent(events[ITEM_ENQUEUED_NOTIFICATION_EVENT]);
    }

    return workerItem;
}


DWORD 
ThreadPool::DoWork(
    _In_ PVOID Context
)
{
    auto threadPool = reinterpret_cast<ThreadPool*>(Context);

    while (!threadPool->pendingShutdown)
    {
        auto item = threadPool->GetWorkerItem();
        if (item)
        {
            item->callback(item->context);
            continue;
        }

        WaitForMultipleObjects(
            static_cast<DWORD>(threadPool->events.size()),
            &threadPool->events[0],
            false,
            INFINITE
        );
    }

    auto item = threadPool->GetWorkerItem();
    while (item)
    {
        item->callback(item->context);
        item = threadPool->GetWorkerItem();
    }

    return 0;
}
