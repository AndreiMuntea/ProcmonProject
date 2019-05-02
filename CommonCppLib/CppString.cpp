#include "include\CppString.hpp"
#include "include\CppAlgorithm.hpp"


Cpp::String::String()
{
    Validate();
}

Cpp::String::String(
    const unsigned __int8* Buffer, 
    unsigned __int32 BufferSize
)
{
    ReplaceBuffer(Buffer, BufferSize) ? Validate() 
                                      : Invalidate();
}

Cpp::String::~String()
{
    DisposeBuffer();
}

Cpp::String::String(const String& Other)
{
    if (!Other.IsValid())
    {
        Invalidate();
        return;
    }

    ReplaceBuffer(Other.buffer, Other.size) ? Validate()
                                            : Invalidate();
}

Cpp::String::String(String&& Other)
{
    if (!Other.IsValid())
    {
        Invalidate();
        return;
    }

    DisposeBuffer();
    Cpp::Swap(this->buffer, Other.buffer);
    Cpp::Swap(this->size, Other.size);

    Validate();
}

Cpp::String& Cpp::String::operator=(const String& Other)
{
    if (!IsValid() || !Other.IsValid())
    {
        Invalidate();
        return *this;
    }

    ReplaceBuffer(Other.buffer, Other.size) ? Validate()
                                            : Invalidate();
    return *this;
}

Cpp::String& Cpp::String::operator=(String&& Other)
{
    if (!IsValid() || !Other.IsValid())
    {
        Invalidate();
        return *this;
    }

    DisposeBuffer();

    Cpp::Swap(this->buffer, Other.buffer);
    Cpp::Swap(this->size, Other.size);
    
    Validate();
    return *this;
}

Cpp::String& Cpp::String::operator+=(const String & Other)
{
    unsigned __int32 newSize = this->size + Other.size;
    if (newSize < this->size || !Other.IsValid())
    {
        this->Invalidate();
        return *this;
    }

    unsigned __int8* temp = (unsigned __int8*)Cpp::LibAlloc(newSize);
    if (!temp)
    {
        this->Invalidate();
        return *this;
    }

    LibCopyMemory(temp, this->buffer, this->size);
    LibCopyMemory(temp + this->size, Other.buffer, Other.size);

    DisposeBuffer();
    this->buffer = temp;
    this->size = newSize;

    return *this;
}

unsigned __int32 
Cpp::String::Count(unsigned __int8 Character) const
{
    unsigned __int32 counter = 0;
    for (unsigned __int32 i = 0; i < this->size; ++i)
    {
        if (this->buffer[i] == Character)
        {
            ++counter;
        }
    }
    return counter;
}

unsigned __int8*
Cpp::String::GetNakedPointer()
{
    return this->buffer;
}

unsigned __int32 
Cpp::String::GetSize() const
{
    return this->size;
}

void
Cpp::String::DisposeBuffer()
{
    if (this->buffer)
    {
        Cpp::LibFree(this->buffer);
        this->buffer = nullptr;
        this->size = 0;
    }
}

bool 
Cpp::String::ReplaceBuffer(
    const unsigned __int8* Buffer, 
    unsigned __int32 BufferSize
)
{
    DisposeBuffer();
    if (!Buffer)
    {
        return true;
    }

    this->buffer = (unsigned __int8*)Cpp::LibAlloc(BufferSize);
    if (!this->buffer)
    {
        return false;
    }

    Cpp::LibCopyMemory(this->buffer, Buffer, BufferSize);
    this->size = BufferSize;

    return true;
}

bool 
Cpp::String::operator==(const String& Other) const
{
    if (!this->IsValid() || !Other.IsValid())
    {
        return false;
    }

    if (this->size != Other.size)
    {
        return false;
    }

    if (this->size == 0)
    {
        return true;
    }

    for (unsigned __int32 i = 0; i < this->size; ++i)
    {
        unsigned __int8 x = this->buffer[i];
        unsigned __int8 y = Other.buffer[i];

        if (x >= 'A' && x <= 'Z')
        {
            x ^= 0x20;
        }

        if (y >= 'A' && y <= 'Z')
        {
            y ^= 0x20;
        }

        if (x != y)
        {
            return false;
        }
    }

    return true;
}

Cpp::Stream& 
Cpp::operator<<(
    Stream& Stream, 
    const String& String
)
{
    if (!String.IsValid())
    {
        Stream.Invalidate();
        return Stream;
    }

    Stream << String.size;
    if (String.size > 0)
    {
        Stream.Write(String.buffer, String.size);
    }

    return Stream;
}

Cpp::Stream& Cpp::operator>>(
    Stream& Stream, 
    String& String
)
{
    String.DisposeBuffer();

    Stream >> String.size;
    if (!Stream.IsValid())
    {
        String.Invalidate();
        return Stream;
    }

    String.buffer = (unsigned __int8*)Cpp::LibAlloc(String.size);
    if (!String.buffer)
    {
        String.Invalidate();
        Stream.Invalidate();
        return Stream;
    }

    if (String.size > 0)
    {
        Stream.Read(String.buffer, String.size);
    }
    
    if (!Stream.IsValid())
    {
        String.Invalidate();
    }

    return Stream;
}
