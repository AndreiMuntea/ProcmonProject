#ifndef __FLT_PORT_MESSAGE_HPP__ 
#define __FLT_PORT_MESSAGE_HPP__

#include <CppStream.hpp>

namespace KmUmShared
{
    enum class MessageCode
    {
        msgProcessCreate = 0,
        msgProcessTerminate = 1,
        msgThreadCreate = 2,
        msgThreadTerminate = 3,
        msgMaxValue,
    };

    class FilterReplyHeader
    {
    public:
        FilterReplyHeader() = default;
        FilterReplyHeader(long ReturnStatus);
        virtual ~FilterReplyHeader() = default;

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const FilterReplyHeader& FilterReplyHeader);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, FilterReplyHeader& FilterReplyHeader);

        long GetReturnStatus() const;

    private:
        long returnStatus = -1;
    };

    class FilterMessageHeader
    {
    public:
        FilterMessageHeader() = default;
        FilterMessageHeader(const MessageCode& MessageCode);
        virtual ~FilterMessageHeader() = default;

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const FilterMessageHeader& MessageHeader);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, FilterMessageHeader& MessageHeader);

        MessageCode GetMessageCode() const;

    private:
        MessageCode messageCode = MessageCode::msgMaxValue;
    };
}

#endif //__FLT_PORT_MESSAGE_HPP__