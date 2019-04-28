#ifndef __CPP_SHALLOW_STREAM_HPP__ 
#define __CPP_SHALLOW_STREAM_HPP__

#include "CppStream.hpp"

namespace Cpp
{
    class ShallowStream : public Stream
    {
    public:
        ShallowStream(unsigned __int8* Buffer, unsigned __int32 BufferSize);
        virtual ~ShallowStream();
    };
}

#endif //__CPP_SHALLOW_STREAM_HPP__