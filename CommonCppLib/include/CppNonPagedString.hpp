#ifndef __CPP_NON_PAGED_STRING_HPP__ 
#define __CPP_NON_PAGED_STRING_HPP__

#include "CppString.hpp"

namespace Cpp
{
    class NonPagedString : public Object
    {
    public:
        NonPagedString();
        NonPagedString(const unsigned __int8* Buffer, unsigned __int32 BufferSize);

        friend Stream& operator<<(Stream& Stream, const NonPagedString& NonPagedString);
        friend Stream& operator>>(Stream& Stream, NonPagedString& NonPagedString);

        ~NonPagedString();

        NonPagedString(const NonPagedString& Other);
        NonPagedString(NonPagedString&& Other);

        NonPagedString& operator=(const NonPagedString& Other);
        NonPagedString& operator=(NonPagedString&& Other);

        NonPagedString& operator+=(const NonPagedString& Other);
        bool operator==(const NonPagedString& Other) const;

        unsigned __int32
        Count(unsigned __int8 Character) const;

        unsigned __int8* GetNakedPointer();
        unsigned __int32 GetSize() const;

    private:
        unsigned __int32 size = 0;
        unsigned __int8* buffer = nullptr;

        void DisposeBuffer();
        bool ReplaceBuffer(const unsigned __int8* Buffer, unsigned __int32 BufferSize);
    };
}

#endif //__CPP_NON_PAGED_STRING_HPP__