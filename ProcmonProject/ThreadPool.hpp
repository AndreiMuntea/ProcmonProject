#ifndef __THREAD_POOL_HPP__ 
#define __THREAD_POOL_HPP__

#include <Windows.h>
#include <mutex>
#include <memory>
#include <queue>
#include <functional>
#include <vector>


class ThreadContext
{
public:
    ThreadContext() = default;
    virtual ~ThreadContext() = default;
};

class ThreadWorkerItem
{
    friend class ThreadPool;
public:
    ThreadWorkerItem(
        _In_opt_ std::shared_ptr<ThreadContext> Context,
        _In_ std::function<void(std::shared_ptr<ThreadContext>)> Callback
    );

    ~ThreadWorkerItem() = default;

private:
    std::shared_ptr<ThreadContext> context;
    std::function<void(std::shared_ptr<ThreadContext>)> callback;
};

class ThreadPool
{
public:
    ThreadPool(_In_ uint8_t NoThreads);
    ~ThreadPool();

    void Shutdown();

    void EnqueueItem(
        _In_opt_ std::shared_ptr<ThreadContext> Context,
        _In_ std::function<void(std::shared_ptr<ThreadContext>)> Callback
    );

    size_t GetNoThreads() const;

private:
    void WaitThreads();
    void DisposeEvents();
    void CleanupThreadPool();
    std::shared_ptr<ThreadWorkerItem> GetWorkerItem();

    static DWORD WINAPI DoWork(_In_ PVOID Context);

    bool pendingShutdown = false;
    std::vector<HANDLE> threads;
    std::vector<HANDLE> events;
    std::mutex itemsLock;
    std::queue<std::shared_ptr<ThreadWorkerItem>> items;
};

#endif //__THREAD_POOL_HPP__