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
    unsigned __int8* ApplicationIdData,
    unsigned __int32 ApplicationIdSize,
    unsigned __int64 LocalAddress,
    unsigned __int64 RemoteAddress,
    unsigned __int64 LocalPort,
    unsigned __int64 RemotePort,
    unsigned __int64 Protocol,
    unsigned __int64 Icmp
) : applicationId{ ApplicationIdData, ApplicationIdSize },
    localAddress{LocalAddress},
    remoteAddress{RemoteAddress},
    localPort{LocalPort},
    remotePort{RemotePort},
    protocol{Protocol},
    icmp{Icmp}
{
}
