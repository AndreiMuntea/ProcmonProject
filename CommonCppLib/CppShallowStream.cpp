#include "include\CppShallowStream.hpp"

Cpp::ShallowStream::ShallowStream(unsigned __int8 * Buffer, unsigned __int32 BufferSize)
{
    this->buffer = Buffer;
    this->size = BufferSize;
    this->capacity = BufferSize;
    this->cursor = 0;

    Validate();
}

Cpp::ShallowStream::~ShallowStream()
{
    this->buffer = nullptr;
    this->size = 0;
    this->capacity = 0;
    this->cursor = 0;
}
