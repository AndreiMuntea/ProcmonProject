#ifndef __CPP_LOCKGUARD_HPP__ 
#define __CPP_LOCKGUARD_HPP__

#include "cpp_lock.hpp"

namespace Cpp
{
    class ExclusiveLockguard
    {
    public:
        explicit ExclusiveLockguard(Lock* Mutex) : mutex{ Mutex }
        {
            this->mutex->Acquire();
        }

        virtual ~ExclusiveLockguard()
        {
            this->mutex->Release();
        }

    private:
        Lock* mutex;
    };

    class SharedLockguard
    {
    public:
        explicit SharedLockguard(SharedLock* Mutex) : mutex{ Mutex }
        {
            this->mutex->AcquireShared();
        }

        virtual ~SharedLockguard()
        {
            this->mutex->ReleaseShared();
        }

    private:
        SharedLock* mutex;
    };
}
#endif //__CPP_LOCKGUARD_HPP__