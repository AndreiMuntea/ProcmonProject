#ifndef __CPP_UNIQUE_POINTER_HPP__ 
#define __CPP_UNIQUE_POINTER_HPP__ 

namespace Cpp
{
    template<class T>
    struct DefaultDeleter
    {
        void operator()(T* Block) { delete Block; }
    };
}

namespace Cpp
{
    template<typename T, class D = DefaultDeleter<T>>
    class UniquePointer
    {
    public:
        UniquePointer() = default;
        UniquePointer(T* Pointer);

        virtual ~UniquePointer();

        UniquePointer(UniquePointer<T, D>&&) = delete;
        UniquePointer<T, D>& operator=(UniquePointer<T, D>&&) = delete;

        UniquePointer(const UniquePointer<T, D>&) = delete;
        UniquePointer<T, D>& operator=(const UniquePointer<T, D>&) = delete;

        void Update(T* NewPointer = nullptr);
        bool IsValid() const;

        T* GetRawPointer();
        T& operator*();
        T* operator->();

    private:
        T* rawPointer = nullptr;
    };

    template<typename T, class D>
    inline UniquePointer<T, D>::UniquePointer(T * Pointer) :
        rawPointer{ Pointer }
    {
    }

    template<typename T, class D>
    inline UniquePointer<T, D>::~UniquePointer()
    {
        if (rawPointer)
        {
            D()(rawPointer);
            rawPointer = nullptr;
        }
    }

    template<typename T, class D>
    inline void UniquePointer<T, D>::Update(T * NewPointer)
    {
        UniquePointer<T, D> temp(this->rawPointer);
        this->rawPointer = NewPointer;
    }

    template<typename T, class D>
    inline bool UniquePointer<T, D>::IsValid() const
    {
        return rawPointer != nullptr;
    }

    template<typename T, class D>
    inline T * UniquePointer<T, D>::GetRawPointer()
    {
        return rawPointer;
    }

    template<typename T, class D>
    inline T & UniquePointer<T, D>::operator*()
    {
        return *rawPointer;
    }
    template<typename T, class D>
    inline T * UniquePointer<T, D>::operator->()
    {
        return rawPointer;
    }
}

#endif //__CPP_UNIQUE_POINTER_HPP__