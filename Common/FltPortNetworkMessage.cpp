#include "FltPortNetworkMessage.hpp"

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const NetworkMessage & NetworkMessage)
{
    Stream << KmUmShared::FilterMessageHeader{ MessageCode::msgNetworkNotification }
        << NetworkMessage.applicationId
        << NetworkMessage.icmp
        << NetworkMessage.localAddress
        << NetworkMessage.localPort
        << NetworkMessage.protocol
        << NetworkMessage.remoteAddress
        << NetworkMessage.remotePort;

    return Stream;
}

Cpp::Stream & KmUmShared::operator >> (Cpp::Stream & Stream, NetworkMessage & NetworkMessage)
{
    Stream >> NetworkMessage.applicationId
        >> NetworkMessage.icmp
        >> NetworkMessage.localAddress
        >> NetworkMessage.localPort
        >> NetworkMessage.protocol
        >> NetworkMessage.remoteAddress
        >> NetworkMessage.remotePort;

    return Stream;
}

KmUmShared::NetworkMessage::NetworkMessage(
    Cpp::NonPagedString ApplicationId,
    unsigned __int32 LocalAddress,
    unsigned __int32 RemoteAddress,
    unsigned __int16 LocalPort,
    unsigned __int16 RemotePort,
    unsigned __int8 Protocol,
    unsigned __int16 Icmp
) : applicationId{ApplicationId},
    localAddress{LocalAddress},
    remoteAddress{RemoteAddress},
    localPort{LocalPort},
    remotePort{RemotePort},
    protocol{Protocol},
    icmp{Icmp}
{
}
