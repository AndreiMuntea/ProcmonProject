#ifndef __FILTER_PORT_HPP__
#define __FILTER_PORT_HPP__

#include "FltPortSerializers.hpp"
#include "../Common/FltPortCommand.hpp"

#include <memory>
#include <string>
#include <thread>
#include <Windows.h>
#include <fltUser.h>
#include <fstream>
#include <CppShallowStream.hpp>

class FilterPort
{
public:
    FilterPort(
        const std::wstring& PortName
    );

    ~FilterPort();

    NTSTATUS Send(
        _In_ std::shared_ptr<KmUmShared::CommandHeader> Command,
        _In_ std::shared_ptr<KmUmShared::CommandReply> Reply
    );

    void Disconnect();
    bool IsConnected();

private:
    void CloseConnectionPort();
    void Listen();

    NTSTATUS
    HandleMessage(
        Cpp::ShallowStream& InputData,
        Cpp::Stream& OutputStream
    );

    template <class T>
    NTSTATUS HandleMessageNotification(
        Cpp::ShallowStream & InputData,
        HANDLE ProcessId,
        Cpp::String& ProcessName,
        unsigned __int64 Timestamp
    );

    std::wofstream log;
    bool isConnected = false;
    std::wstring portName;
    std::unique_ptr<std::thread> listenerThread = nullptr;
    HANDLE driverPort = INVALID_HANDLE_VALUE;
};


template<class T>
inline NTSTATUS FilterPort::HandleMessageNotification(
    Cpp::ShallowStream & InputData,
    HANDLE ProcessId,
    Cpp::String& ProcessName,
    unsigned __int64 Timestamp
)
{
    T message;
    InputData >> message;

    if (!InputData.IsValid())
    {
        return ERROR_INVALID_PARAMETER;
    }

    // ignore writes from this process (logs)
    if (GetCurrentProcessId() == (SIZE_T)ProcessId && std::is_same<T, KmUmShared::FileWriteMessage>::value)
    {
        return ERROR_SUCCESS;
    }

    log << message
        << "\t> [Process Id] " << HandleToULong(ProcessId) << std::endl
        << "\t> [Process Name] " << ProcessName << std::endl
        << "\t> [Timestamp] " << Timestamp << std::endl
        << std::endl;


    return ERROR_SUCCESS;
}

#endif //__FILTER_PORT_HPP__
