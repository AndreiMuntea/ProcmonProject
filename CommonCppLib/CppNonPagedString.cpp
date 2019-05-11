#include "include\CppNonPagedString.hpp"
#include "include\CppAlgorithm.hpp"

Cpp::NonPagedString::NonPagedString()
{
    Validate();
}

Cpp::NonPagedString::NonPagedString(
    const unsigned __int8* Buffer,
    unsigned __int32 BufferSize
)
{
    ReplaceBuffer(Buffer, BufferSize) ? Validate()
                                      : Invalidate();
}

Cpp::NonPagedString::~NonPagedString()
{
    DisposeBuffer();
}

Cpp::NonPagedString::NonPagedString(const NonPagedString& Other)
{
    if (!Other.IsValid())
    {
        Invalidate();
        return;
    }

    ReplaceBuffer(Other.buffer, Other.size) ? Validate()
                                            : Invalidate();
}

Cpp::NonPagedString::NonPagedString(NonPagedString&& Other)
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

Cpp::NonPagedString& Cpp::NonPagedString::operator=(const NonPagedString& Other)
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

Cpp::NonPagedString& Cpp::NonPagedString::operator=(NonPagedString&& Other)
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

Cpp::NonPagedString& Cpp::NonPagedString::operator+=(const NonPagedString & Other)
{
    unsigned __int32 newSize = this->size + Other.size;
    if (newSize < this->size || !Other.IsValid())
    {
        this->Invalidate();
        return *this;
    }

    unsigned __int8* temp = (unsigned __int8*)Cpp::LibAllocNonpaged(newSize);
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
Cpp::NonPagedString::Count(unsigned __int8 Character) const
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
Cpp::NonPagedString::GetNakedPointer()
{
    return this->buffer;
}

unsigned __int32
Cpp::NonPagedString::GetSize() const
{
    return this->size;
}

void
Cpp::NonPagedString::DisposeBuffer()
{
    if (this->buffer)
    {
        Cpp::LibFreeNonpaged(this->buffer);
        this->buffer = nullptr;
        this->size = 0;
    }
}

bool
Cpp::NonPagedString::ReplaceBuffer(
    const unsigned __int8* Buffer,
    unsigned __int32 BufferSize
)
{
    DisposeBuffer();
    if (!Buffer)
    {
        return true;
    }

    this->buffer = (unsigned __int8*)Cpp::LibAllocNonpaged(BufferSize);
    if (!this->buffer)
    {
        return false;
    }

    Cpp::LibCopyMemory(this->buffer, Buffer, BufferSize);
    this->size = BufferSize;

    return true;
}

bool
Cpp::NonPagedString::operator==(const NonPagedString& Other) const
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
Cpp::Stream & Cpp::operator<<(Stream & Stream, const NonPagedString & NonPagedString)
{
    if (!NonPagedString.IsValid())
    {
        Stream.Invalidate();
        return Stream;
    }

    Stream << NonPagedString.size;
    if (NonPagedString.size > 0)
    {
        Stream.Write(NonPagedString.buffer, NonPagedString.size);
    }

    return Stream;
}

Cpp::Stream & Cpp::operator >> (Stream & Stream, NonPagedString & NonPagedString)
{
    NonPagedString.DisposeBuffer();

    Stream >> NonPagedString.size;
    if (!Stream.IsValid())
    {
        NonPagedString.Invalidate();
        return Stream;
    }

    NonPagedString.buffer = (unsigned __int8*)Cpp::LibAllocNonpaged(NonPagedString.size);
    if (!NonPagedString.buffer)
    {
        NonPagedString.Invalidate();
        Stream.Invalidate();
        return Stream;
    }

    if (NonPagedString.size > 0)
    {
        Stream.Read(NonPagedString.buffer, NonPagedString.size);
    }

    if (!Stream.IsValid())
    {
        NonPagedString.Invalidate();
    }

    return Stream;
}
