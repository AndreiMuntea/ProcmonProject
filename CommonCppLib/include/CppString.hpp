#ifndef __CPP_STRING_HPP__ 
#define __CPP_STRING_HPP__

#include "CppExternals.hpp"
#include "CppObject.hpp"
#include "CppStream.hpp"

namespace Cpp
{
    class String : public Object
    {
    public:
        String();
        String(const unsigned __int8* Buffer, unsigned __int32 BufferSize);

        ~String();

        String(const String& Other);
        String(String&& Other);

        String& operator=(const String& Other);
        String& operator=(String&& Other);

        String& operator+=(const String& Other);

        friend Stream& operator<<(Stream& Stream, const String& String);
        friend Stream& operator>>(Stream& Stream, String& String);

        unsigned __int8* GetNakedPointer();
        unsigned __int32 GetSize() const;

    private:
        unsigned __int32 size = 0;
        unsigned __int8* buffer = nullptr;

        void DisposeBuffer();
        bool ReplaceBuffer(const unsigned __int8* Buffer, unsigned __int32 BufferSize);
    };
}

#endif //__CPP_UNICODE_STRING_HPP__