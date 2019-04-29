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
        msgModuleLoaded = 4,
        msgRegistryCreate = 5,
        msgRegistrySetValue = 6,
        msgRegistryDeleteKey = 7,
        msgRegistryDeleteValue = 8,
        msgRegistryLoadKey = 9,
        msgRegistryRenameKey = 10,
        msgFileCreate = 11,
        msgFileClose = 12,
        msgFileCleanup = 13,
        msgFileRead = 14,
        msgFileWrite = 15,
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