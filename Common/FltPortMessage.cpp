#include "FltPortMessage.hpp"

KmUmShared::FilterMessageHeader::FilterMessageHeader(
    const MessageCode& MessageCode
) : messageCode{MessageCode}
{
}

KmUmShared::MessageCode 
KmUmShared::FilterMessageHeader::GetMessageCode() const
{
    return messageCode;
}

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const FilterReplyHeader & FilterReplyHeader)
{
    Stream << FilterReplyHeader.returnStatus;
    return Stream;
}

Cpp::Stream & KmUmShared::operator >> (Cpp::Stream & Stream, FilterReplyHeader & FilterReplyHeader)
{
    Stream >> FilterReplyHeader.returnStatus;
    return Stream;
}

Cpp::Stream&
KmUmShared::operator<<(
    Cpp::Stream& Stream, 
    const FilterMessageHeader& MessageHeader
)
{
    Stream << static_cast<unsigned __int32>(MessageHeader.messageCode);
    return Stream;
}

Cpp::Stream& 
KmUmShared::operator>>(
    Cpp::Stream& Stream, 
    FilterMessageHeader& MessageHeader
)
{
    unsigned __int32 code = static_cast<unsigned __int32>(MessageCode::msgMaxValue);
    Stream >> code;

    MessageHeader.messageCode = static_cast<MessageCode>(code);

    return Stream;
}

KmUmShared::FilterReplyHeader::FilterReplyHeader(
    long ReturnStatus
) : returnStatus {ReturnStatus}
{
}

long KmUmShared::FilterReplyHeader::GetReturnStatus() const
{
    return returnStatus;
}
