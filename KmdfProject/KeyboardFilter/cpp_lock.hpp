#ifndef __CPP_LOCK_HPP__ 
#define __CPP_LOCK_HPP__

#include <ntddk.h>
#include <wdf.h>

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

};

#endif //__CPP_LOCK_HPP__