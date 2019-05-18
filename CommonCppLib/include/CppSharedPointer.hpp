#ifndef __CPP_SHARED_POINTER_HPP__ 
#define __CPP_SHARED_POINTER_HPP__ 

#include "CppDefaultDeleter.hpp"
#include "CppAlgorithm.hpp"
#include "CppExternals.hpp"
#include "CppDefaultDeleter.hpp"
#include <intrin.h>

namespace Cpp
{
    template<typename T, class D = DefaultDeleter<T>>
    class SharedPointer
    {
    public:
        SharedPointer() = default;
        SharedPointer(T* Pointer);

        virtual ~SharedPointer();

        SharedPointer(SharedPointer<T, D>&&) = delete;
        SharedPointer<T, D>& operator=(SharedPointer<T, D>&&) = delete;

        SharedPointer(SharedPointer<T, D>& Other);
        SharedPointer<T, D>& operator=(SharedPointer<T, D>& Other);

        void Update(T* NewPointer = nullptr);
        bool IsValid() const;

        T* GetRawPointer();
        T& operator*();
        T* operator->();
    private:
        long* InitializeNewReferenceCounter();
        void UninitializeReferenceCounter(long** ReferenceCounter);

        long Reference();
        long Dereference();

        long* referenceCounter = nullptr;
        T* rawPointer = 0;
    };

    template<typename T, class D>
    inline SharedPointer<T, D>::SharedPointer(T * Pointer)
    {
        referenceCounter = InitializeNewReferenceCounter();
        if (referenceCounter)
        {
            this->rawPointer = Pointer;
        }
        else if (Pointer)
        {
            D()(Pointer);
        }
    }

    template<typename T, class D>
    inline SharedPointer<T, D>::~SharedPointer()
    {
        if (Dereference() != 0)
        {
            return;
        }

        Cpp::LibFreeNonpaged(referenceCounter);
        D()(rawPointer);

        rawPointer = nullptr;
        referenceCounter = nullptr;
    }

    template<typename T, class D>
    inline SharedPointer<T, D>::SharedPointer(SharedPointer<T, D>& Other)
    {
        Other.Reference();

        this->rawPointer = Other.rawPointer;
        this->referenceCounter = Other.referenceCounter;
    }

    template<typename T, class D>
    inline SharedPointer<T, D>& SharedPointer<T, D>::operator=(SharedPointer<T, D>& Other)
    {
        SharedPointer<T, D> temp;
        Cpp::Swap(this->rawPointer, temp.rawPointer);
        Cpp::Swap(this->referenceCounter, temp.referenceCounter);

        Other.Reference();
        this->rawPointer = Other.rawPointer;
        this->referenceCounter = Other.referenceCounter;
    }

    template<typename T, class D>
    inline void SharedPointer<T, D>::Update(T * NewPointer)
    {
        SharedPointer<T, D> temp;
        Cpp::Swap(this->rawPointer, temp.rawPointer);
        Cpp::Swap(this->referenceCounter, temp.referenceCounter);

        this->referenceCounter = InitializeNewReferenceCounter();
        if (this->referenceCounter)
        {
            this->rawPointer = NewPointer;
        }
        else if (NewPointer)
        {
            D()(NewPointer);
        }
    }

    template<typename T, class D>
    inline bool SharedPointer<T, D>::IsValid() const
    {
        return (rawPointer != nullptr) && (referenceCounter != nullptr) && (*referenceCounter > 0);
    }

    template<typename T, class D>
    inline T * SharedPointer<T, D>::GetRawPointer()
    {
        return rawPointer;
    }

    template<typename T, class D>
    inline T & SharedPointer<T, D>::operator*()
    {
        return *rawPointer;
    }

    template<typename T, class D>
    inline T * SharedPointer<T, D>::operator->()
    {
        return rawPointer;
    }

    template<typename T, class D>
    inline long * SharedPointer<T, D>::InitializeNewReferenceCounter()
    {
        auto result = (long*)Cpp::LibAllocNonpaged(sizeof(long));
        if (result)
        {
            *result = 1;
        }
        return result;
    }

    template<typename T, class D>
    inline void SharedPointer<T, D>::UninitializeReferenceCounter(long ** ReferenceCounter)
    {
        if (*result)
        {
            Cpp::LibFreeNonpaged(*ReferenceCounter);
            *ReferenceCounter = nullptr;
        }
    }

    template<typename T, class D>
    inline long SharedPointer<T, D>::Reference()
    {
        long value = -1;
        if (referenceCounter)
        {
            value = _InterlockedIncrement(referenceCounter);
        }
        return value;
    }

    template<typename T, class D>
    inline long SharedPointer<T, D>::Dereference()
    {
        long value = -1;
        if (referenceCounter)
        {
            value = _InterlockedDecrement(referenceCounter);
        }
        return value;
    }

}

#endif//__CPP_SHARED_POINTER_HPP__