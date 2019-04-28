#include "include\CppAlgorithm.hpp"
#include "include\CppStream.hpp"

Cpp::Stream::Stream()
{
    Validate();
}

Cpp::Stream::Stream(unsigned __int32 Capacity)
{
    this->buffer = (unsigned __int8*)LibAlloc(Capacity);
    if (!this->buffer)
    {
        Invalidate();
        return;
    }

    this->capacity = Capacity;
}

Cpp::Stream::~Stream()
{
    if (buffer)
    {
        Cpp::LibFree(buffer);
        buffer = nullptr;
    }
}

Cpp::Stream::Stream(Stream && Other)
{
    Cpp::Swap(this->capacity, Other.capacity);
    Cpp::Swap(this->size, Other.size);
    Cpp::Swap(this->cursor, Other.cursor);
    Cpp::Swap(this->buffer, Other.buffer);
    Cpp::Swap(this->isValid, Other.isValid);
}

bool 
Cpp::Stream::EnsureCapacity(
    unsigned __int32 NeededSize
)
{
    unsigned __int32 newSize = this->size + NeededSize;

    // Overflow
    if (newSize < this->size)
    {
        return false;
    }

    if (newSize < this->capacity)
    {
        return true;
    }

    unsigned __int32 newCapacity = Cpp::Max(newSize, this->capacity * 2);
    return Resize(newCapacity);
}

bool 
Cpp::Stream::Resize(
    unsigned __int32 NewCapacity
)
{
    unsigned __int8* tempBuffer = (unsigned __int8*)Cpp::LibAlloc(NewCapacity);
    if (!tempBuffer)
    {
        return false;
    }

    if (buffer)
    {
        Cpp::LibCopyMemory(tempBuffer, buffer, this->size);
        Cpp::LibFree(buffer);
    }

    buffer = tempBuffer;
    this->capacity = NewCapacity;

    return true;
}

void
Cpp::Stream::Write(
    unsigned __int8 * Buffer, 
    unsigned __int32 BufferSize
)
{
    if (!IsValid() || !EnsureCapacity(BufferSize))
    {
        Invalidate();
        return;
    }

    Cpp::LibCopyMemory(this->buffer + this->size, Buffer, BufferSize);
    this->size += BufferSize;
}

void 
Cpp::Stream::Read(
    unsigned __int8 * Buffer, 
    unsigned __int32 BufferSize
)
{
    unsigned __int32 newCursor = this->cursor + BufferSize;
    if (!IsValid() || newCursor < this->cursor || newCursor >= this->capacity)
    {
        Invalidate();
        return;
    }

    Cpp::LibCopyMemory(Buffer, this->buffer + this->cursor, BufferSize);
    this->cursor = newCursor;
}

unsigned __int8 * Cpp::Stream::GetRawData()
{
    return buffer;
}

unsigned __int32 Cpp::Stream::GetSize() const
{
    return size;
}