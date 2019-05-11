#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "NetworkManager.hpp"

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <fwpmu.h>
#include <cguid.h>

#include <iostream>


NetworkManager::NetworkManager()
{
    auto returnValue = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (returnValue != 0)
    {
        std::wcout << "WSAStartup failed with status " << std::hex << returnValue << std::dec << std::endl;
        throw std::exception("WSAStartup failed");
    }

    returnValue = FwpmEngineOpen(nullptr, RPC_C_AUTHN_WINNT, nullptr, nullptr, &engineHandle);
    if (returnValue != ERROR_SUCCESS)
    {
        WSACleanup();
        std::wcout << "FwpmEngineOpen0 failed with status " << std::hex << returnValue << std::dec << std::endl;
        throw std::exception("FwpmEngineOpen0 failed");
    }
}

NetworkManager::~NetworkManager()
{
    for (auto& filter : filters)
    {
        auto filterId = std::get<0>(filter.second);

        auto result = FwpmFilterDeleteById(engineHandle, filterId);
        if (result != ERROR_SUCCESS)
        {
            std::wcout << "FwpmFilterDeleteById failed with Error " << std::hex << result << std::dec << std::endl;
        }

    }

    FwpmEngineClose(engineHandle);
    WSACleanup();
}

void NetworkManager::BlockAccess(const std::wstring & Website, const std::wstring & Application)
{
    auto ipAddresses = ListAddrInfo(Website);
    for (auto& ipAddress : ipAddresses)
    {
        UINT64 filterId = 0;
        auto filter = std::make_shared<Filter>(ipAddress, UINT32_MAX, Application);
        
        auto result = FwpmFilterAdd0(this->engineHandle, filter->GetFilter(), nullptr, &filterId);
        if (result != ERROR_SUCCESS)
        {
            std::wcout << "FwpmFilterAdd0 failed with status " << std::hex << result << std::dec << std::endl;
        }
        else
        {
            this->filters[std::make_tuple(Website, Application)] = std::make_tuple(filterId, filter);
        }
    }
}

void NetworkManager::UnblockAccess(const std::wstring & Website, const std::wstring & Application)
{
    auto& filter = this->filters[std::make_tuple(Website, Application)];
    auto filterId = std::get<0>(filter);

    auto result = FwpmFilterDeleteById(engineHandle, filterId);
    if (result != ERROR_SUCCESS)
    {
        std::wcout << "FwpmFilterDeleteById failed with Error " << std::hex << result << std::dec << std::endl;
    }

    this->filters.erase(std::make_tuple(Website, Application));
}

UINT32 NetworkManager::ExtractIpAddress(ADDRINFOW * AddrInfo)
{
    auto addr_in = (struct sockaddr_in*)(AddrInfo->ai_addr);
    auto address = ntohl(addr_in->sin_addr.S_un.S_addr);
    auto addressString = inet_ntoa(addr_in->sin_addr);

    std::cout << "Ip found: " << addressString << " hex : " << std::hex << address << std::dec << std::endl;
    return address;
}

std::vector<UINT32> NetworkManager::ListAddrInfo(const std::wstring & Website)
{
    ADDRINFOW* result = nullptr;
    ADDRINFOW hints = { 0 };
    std::vector<UINT32> ipAddresses;

    // # <service name>  <port number>/<protocol>  [aliases...]             [#<comment>]
    // http               80/tcp                    www www-http             #World Wide Web

    // https://docs.microsoft.com/en-us/windows/desktop/api/ws2tcpip/nf-ws2tcpip-getaddrinfow
    hints.ai_family = AF_INET;            // Only IpV4 -- if the caller handles only IPv4 and does not handle IPv6, the ai_family member should be set to AF_INET
    hints.ai_socktype = SOCK_STREAM;      // If the caller handles only TCP and does not handle UDP, the ai_socktype member should be set to SOCK_STREAM
    hints.ai_protocol = IPPROTO_TCP;      // Only TCP protocol

    auto returnValue = GetAddrInfoW(Website.c_str(), nullptr, &hints, &result);
    if (returnValue != 0)
    {
        std::wcout << "GetAddrInfoW failed with status " << std::hex << returnValue << std::dec << std::endl;
        throw std::exception("GetAddrInfoW failed");
    }

    for (auto currentAddrInfo = result; currentAddrInfo != nullptr; currentAddrInfo = result->ai_next)
    {
        if (currentAddrInfo->ai_family != AF_INET)
        {
            continue;
        }
        if (currentAddrInfo->ai_socktype != SOCK_STREAM)
        {
            continue;
        }
        if (currentAddrInfo->ai_protocol != IPPROTO_TCP)
        {
            continue;
        }

        auto ip = ExtractIpAddress(currentAddrInfo);
        if (ip != 0)
        {
            ipAddresses.push_back(ip);
        }
    }

    FreeAddrInfoW(result);
    return ipAddresses;
}


Filter::Filter(UINT32 Address, UINT32 Mask, const std::wstring& Application)
{
    auto result = FwpmGetAppIdFromFileName(Application.c_str(), &this->applicationBlob);
    if (result != ERROR_SUCCESS)
    {
        std::wcout << "FwpmGetAppIdFromFileName failed with status " << std::hex << result << std::dec << std::endl;
        throw std::exception("FwpmGetAppIdFromFileName failed");
    }

    this->displayDataName = L"Blocks all incoming connection to IP/MASK";
    this->displayDataDescription= L"Blocks all incoming connection to " + std::to_wstring(Address) + L" mask : " + std::to_wstring(Mask);

    this->ipAddressAndMask.addr = Address;
    this->ipAddressAndMask.mask = Mask;

    this->conditions[0].fieldKey = FWPM_CONDITION_IP_REMOTE_ADDRESS;
    this->conditions[0].matchType = FWP_MATCH_EQUAL;
    this->conditions[0].conditionValue.type = FWP_V4_ADDR_MASK;
    this->conditions[0].conditionValue.v4AddrMask = &this->ipAddressAndMask;

    this->conditions[1].fieldKey = FWPM_CONDITION_ALE_APP_ID;
    this->conditions[1].matchType = FWP_MATCH_EQUAL;
    this->conditions[1].conditionValue.type = FWP_BYTE_BLOB_TYPE;
    this->conditions[1].conditionValue.byteBlob = this->applicationBlob;

    this->filter.flags = FWPM_FILTER_FLAG_NONE;
    this->filter.subLayerKey = IID_NULL;
    this->filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;
    this->filter.action.type = FWP_ACTION_BLOCK;
    this->filter.weight.type = FWP_EMPTY;
    this->filter.numFilterConditions = ARRAYSIZE(this->conditions);
    this->filter.filterCondition = this->conditions;
    this->filter.displayData.name = (wchar_t*)this->displayDataName.c_str();
    this->filter.displayData.description = (wchar_t*)this->displayDataDescription.c_str();
}

Filter::~Filter()
{
    FwpmFreeMemory((void**)&this->applicationBlob);
}

const FWPM_FILTER0 * Filter::GetFilter() const
{
    return &filter;
}
