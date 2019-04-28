#ifndef __CPP_LOCK_HPP__ 
#define __CPP_LOCK_HPP__

#include <fltKernel.h>

namespace Cpp
{
    class Lock
    {
    public:
        Lock() = default;
        virtual ~Lock() = default;

        virtual void Acquire() = 0;
        virtual void Release() = 0;
    };

    class SharedLock : public Lock
    {
    public:
        SharedLock() = default;
        virtual ~SharedLock() = default;

        virtual void AcquireShared() = 0;
        virtual void ReleaseShared() = 0;
    };

    class Spinlock : public Lock
    {
    public:
        Spinlock()
        {
            KeInitializeSpinLock(&lock);
        }

        virtual ~Spinlock()
        {
            Acquire();
            Release();
        }

#pragma warning(suppress: 28167)
        virtual void Acquire() override
        {
            KeAcquireSpinLock(&lock, &irql);
        }

#pragma warning(suppress: 28167)
        virtual void Release() override
        {
            KeReleaseSpinLock(&lock, irql);
        }

    private:
        KSPIN_LOCK lock{};
        KIRQL irql{};
    };


    class Pushlock : public SharedLock
    {
    public:
        Pushlock()
        {
            FltInitializePushLock(&lock);
        }

        virtual ~Pushlock()
        {
            Acquire();
            Release();
            FltDeletePushLock(&lock);
        }

        virtual void Acquire() override
        {
            FltAcquirePushLockExclusive(&lock);
        }

        virtual void Release() override
        {
            FltReleasePushLock(&lock);
        }

        virtual void AcquireShared() override
        {
            FltAcquirePushLockShared(&lock);
        }

        virtual void ReleaseShared() override
        {
            FltReleasePushLock(&lock);
        }

    private:
        EX_PUSH_LOCK lock;
    };

    class Eresource : public Lock
    {
    public:
        Eresource()
        {
            auto status = ExInitializeResourceLite(&resource);
            NT_VERIFY(NT_SUCCESS(status));
        }

        virtual ~Eresource()
        {
            Acquire();
            Release();

            auto status = ExDeleteResourceLite(&resource);
            NT_VERIFY(NT_SUCCESS(status));
        }

        virtual void Acquire() override
        {
            ExEnterCriticalRegionAndAcquireResourceExclusive(&resource);
        }

        virtual void Release() override
        {
            ExReleaseResourceAndLeaveCriticalRegion(&resource);
        }

    private:
        ERESOURCE resource{};
    };
};

#endif //__CPP_LOCK_HPP__