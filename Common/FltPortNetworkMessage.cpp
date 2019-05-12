#include "FltPortNetworkMessage.hpp"

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const NetworkMessageIpV4 & NetworkMessageIpV4)
{
    Stream << KmUmShared::FilterMessageHeader{ MessageCode::msgNetworkNotificationIpV4 }
        << NetworkMessageIpV4.applicationId
        << NetworkMessageIpV4.icmp
        << NetworkMessageIpV4.localAddress
        << NetworkMessageIpV4.localPort
        << NetworkMessageIpV4.protocol
        << NetworkMessageIpV4.remoteAddress
        << NetworkMessageIpV4.remotePort;

    return Stream;
}

Cpp::Stream & KmUmShared::operator >> (Cpp::Stream & Stream, NetworkMessageIpV4 & NetworkMessageIpV4)
{
    Stream >> NetworkMessageIpV4.applicationId
        >> NetworkMessageIpV4.icmp
        >> NetworkMessageIpV4.localAddress
        >> NetworkMessageIpV4.localPort
        >> NetworkMessageIpV4.protocol
        >> NetworkMessageIpV4.remoteAddress
        >> NetworkMessageIpV4.remotePort;

    return Stream;
}

KmUmShared::NetworkMessageIpV4::NetworkMessageIpV4(
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
