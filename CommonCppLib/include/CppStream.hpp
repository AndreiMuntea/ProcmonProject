#ifndef __CPP_STREAM_HPP__ 
#define __CPP_STREAM_HPP__

#include "CppObject.hpp"
#include "CppExternals.hpp"

namespace Cpp
{
    class Stream : public Object
    {
    public:
        Stream();
        Stream(unsigned __int32 Capacity);
        virtual ~Stream();

        Stream(const Stream& Other) = delete;
        Stream(Stream&& Other) = delete;

        void operator=(const Stream& Other) = delete;
        void operator=(Stream&& Other) = delete;

        template <class T>
        Stream& operator<<(const T& Data);

        template <class T>
        Stream& operator>>(T& Data);

        void Write(unsigned __int8* Buffer, unsigned __int32 BufferSize);
        void Read(unsigned __int8* Buffer, unsigned __int32 BufferSize);

        unsigned __int8* GetRawData();
        unsigned __int32 GetSize() const;

    protected:
        unsigned __int32 capacity = 0;
        unsigned __int32 size = 0;
        unsigned __int32 cursor = 0;
        unsigned __int8* buffer = nullptr;

        bool EnsureCapacity(unsigned __int32 NeededSize);
        bool Resize(unsigned __int32 NewCapacity);
    };

    template<class T>
    inline Stream& Stream::operator<<(const T & Data)
    {
        Write((unsigned __int8*)(&Data), sizeof(Data));
        return *this;
    }

    template<class T>
    inline Stream& Stream::operator>>(T & Data)
    {
        Read((unsigned __int8*)(&Data), sizeof(Data));
        return *this;
    }
}

#endif //__CPP_STREAM_HPP__