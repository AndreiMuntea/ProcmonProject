#ifndef __FLT_PORT_NETWORK_MESSAGE_HPP__
#define __FLT_PORT_NETWORK_MESSAGE_HPP__

#include <CppString.hpp>
#include <sal.h>

#include "FltPortMessage.hpp"

namespace KmUmShared
{
    class NetworkMessage
    {
    public:
        NetworkMessage() = default;
        NetworkMessage(
            unsigned __int8* ApplicationIdData,
            unsigned __int32 ApplicationIdSize,
            unsigned __int64 LocalAddress,
            unsigned __int64 RemoteAddress,
            unsigned __int64 LocalPort,
            unsigned __int64 RemotePort,
            unsigned __int64 Protocol,
            unsigned __int64 Icmp
        );

        virtual ~NetworkMessage() = default;

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const NetworkMessage& NetworkMessage);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, NetworkMessage& NetworkMessage);

    public:
        Cpp::String applicationId;
        unsigned __int64 localAddress = 0;
        unsigned __int64 remoteAddress = 0;
        unsigned __int64 localPort = 0;
        unsigned __int64 remotePort = 0;
        unsigned __int64 protocol = 0;
        unsigned __int64 icmp = 0;
    };
}

#endif //__FLT_PORT_NETWORK_MESSAGE_HPP__