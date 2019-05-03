#include "Registry.hpp"
#include <iostream>

static void PrintErrorLstatus(LSTATUS status)
{
    LPWSTR lpMsgBuf = nullptr;

    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        status,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&lpMsgBuf,
        0, NULL
    );

    std::wcout << "Error code description: " << std::wstring{ lpMsgBuf } << std::endl;
    LocalFree(lpMsgBuf);
}

RegistryKey::RegistryKey(HKEY Key, const std::wstring & Subkey)
{
    auto status = RegCreateKeyExW(
        Key,
        Subkey.c_str(),
        0,
        nullptr,
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        nullptr,
        &this->key,
        nullptr
    );

    if (status != ERROR_SUCCESS)
    {
        this->key = nullptr;

        std::wcout << "RegCreateKeyExW failed with status = " << std::hex << status << std::dec << std::endl;
        PrintErrorLstatus(status);
        throw std::exception("Failed to create key!");
    }
}

RegistryKey::~RegistryKey()
{
    if (this->key)
    {
        RegCloseKey(this->key);
    }
}

void RegistryKey::SetValue(const unsigned __int64 & Data)
{
    SetValue((PVOID)&Data, sizeof(Data), REG_QWORD);
}

void RegistryKey::GetValue(unsigned __int64 & Data)
{
    GetValue((PVOID)&Data, sizeof(Data), REG_QWORD);
}

void RegistryKey::SetValue(PVOID Data, DWORD DataSize, DWORD Type)
{
    auto status = RegSetValueExW(this->key, nullptr, 0, Type, (const BYTE*)Data, DataSize);
    if (status != ERROR_SUCCESS)
    {
        std::wcout << "RegSetValueExW failed with status = " << std::hex << status << std::dec << std::endl;
        PrintErrorLstatus(status);
    }
}

void RegistryKey::GetValue(PVOID Data, DWORD DataSize, DWORD Type)
{
    ZeroMemory(Data, DataSize);
    DWORD type = Type + 1;
   
    RegQueryValueExW(this->key, nullptr, nullptr, &type, nullptr, nullptr);
    if (type != Type)
    {
        std::wcout << "Different type expected for RegQueryValueExW" << std::endl;
        return;
    }

    DWORD size = DataSize;
    auto status = RegQueryValueExW(this->key, nullptr, nullptr, nullptr, (LPBYTE)Data, &size);
    if (status != ERROR_SUCCESS)
    {
        std::wcout << "RegQueryValueExW failed with status = " << std::hex << status << std::dec << std::endl;
        PrintErrorLstatus(status);
        return;
    }

    if (size != DataSize)
    {
        std::wcout << "Different DataSize expected for RegQueryValueExW" << std::endl;
        return;
    }
}
