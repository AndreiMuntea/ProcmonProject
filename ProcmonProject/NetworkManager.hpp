#ifndef __NETWORK_MANAGER_HPP__ 
#define __NETWORK_MANAGER_HPP__

#include <string>
#include <vector>
#include <map>
#include <memory>

#include <fwpmtypes.h>
#include <WinSock2.h>


class Filter
{
public:
    Filter(UINT32 Address, UINT32 Mask, const std::wstring& Application);
    ~Filter();

    const FWPM_FILTER0 * GetFilter() const;

private:
    FWP_BYTE_BLOB *applicationBlob = nullptr;
    FWPM_FILTER0 filter = { 0 };
    FWPM_FILTER_CONDITION0 conditions[2] = { 0 };
    FWP_V4_ADDR_AND_MASK ipAddressAndMask = { 0 };

    std::wstring displayDataName;
    std::wstring displayDataDescription;
};

class NetworkManager
{
public:
    NetworkManager();
    ~NetworkManager();

    void BlockAccess(const std::wstring& Website, const std::wstring& Application);
    void UnblockAccess(const std::wstring& Website, const std::wstring& Application);

private:
    UINT32 ExtractIpAddress(ADDRINFOW* AddrInfo);
    std::vector<UINT32> ListAddrInfo(const std::wstring& Website);

    WSADATA wsaData = { 0 };
    HANDLE engineHandle = nullptr;
    std::map<std::tuple<std::wstring, std::wstring>, std::tuple<UINT64, std::shared_ptr<Filter>>> filters;
};

#endif //__NETWORK_MANAGER_HPP__