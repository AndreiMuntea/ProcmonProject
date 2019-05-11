#ifndef __FLT_PORT_NETWORK_MESSAGE_HPP__
#define __FLT_PORT_NETWORK_MESSAGE_HPP__

#include <CppNonPagedString.hpp>
#include <sal.h>

#include "FltPortMessage.hpp"


namespace KmUmShared
{
    class NetworkMessage
    {
    public:
        NetworkMessage() = default;
        NetworkMessage(
            Cpp::NonPagedString ApplicationId,
            unsigned __int32 LocalAddress,
            unsigned __int32 RemoteAddress,
            unsigned __int16 LocalPort,
            unsigned __int16 RemotePort,
            unsigned __int8 Protocol,
            unsigned __int16 Icmp
        );

        virtual ~NetworkMessage() = default;

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const NetworkMessage& NetworkMessage);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, NetworkMessage& NetworkMessage);

    public:
        Cpp::NonPagedString applicationId;
        unsigned __int32 localAddress = 0;
        unsigned __int32 remoteAddress = 0;
        unsigned __int16 localPort = 0;
        unsigned __int16 remotePort = 0;
        unsigned __int8 protocol = 0;
        unsigned __int16 icmp = 0;
    };
}

#endif //__FLT_PORT_NETWORK_MESSAGE_HPP__