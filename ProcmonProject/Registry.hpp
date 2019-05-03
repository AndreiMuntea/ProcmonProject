#ifndef __REGISTRY_HPP__ 
#define __REGISTRY_HPP__

#include <Windows.h>
#include <string>

class RegistryKey
{
public:
    RegistryKey(HKEY Key, const std::wstring& Subkey);
    ~RegistryKey();

    void SetValue(const unsigned __int64& Data);
    void GetValue(unsigned __int64& Data);

private:
    void SetValue(PVOID Data, DWORD DataSize, DWORD Type);
    void GetValue(PVOID Data, DWORD DataSize, DWORD Type);
    HKEY key = nullptr;
};

#endif //__REGISTRY_HPP__