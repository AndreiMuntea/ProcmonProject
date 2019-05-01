#include "FilterPort.hpp"
#include "trace.h"
#include "FilterPort.tmh"

#include <iostream>


FilterPort::FilterPort(const std::wstring & PortName) 
    : portName{PortName},
      log{"log.log"}
{
    auto result = FilterConnectCommunicationPort(portName.c_str(), 0, nullptr, 0, nullptr, &this->driverPort);
    if (result != S_OK)
    {
        this->driverPort = INVALID_HANDLE_VALUE;
        std::wcout << "FilterConnectCommunicationPort failed! result=" << result << std::endl;
        ConsoleAppLogError("FilterConnectCommunicationPort failed with result=%d", result);
        throw std::exception("FilterConnectCommunicationPort failed");
    }

    listenerThread.reset(new std::thread([this]() {this->Listen(); }));
    isConnected = true;
}

void FilterPort::Disconnect()
{
    if (driverPort == INVALID_HANDLE_VALUE)
    {
        std::wcout << "Port is already disconnected!" << std::endl;
        ConsoleAppLogError("Port is already disconnected!");
        throw std::exception("Port is already disconnected!");
    }

    CloseConnectionPort();
}

bool FilterPort::IsConnected()
{
    return isConnected;
}

void FilterPort::CloseConnectionPort()
{
    if (INVALID_HANDLE_VALUE != driverPort)
    {
        CloseHandle(driverPort);
        driverPort = INVALID_HANDLE_VALUE;
    }

    isConnected = false;
}

void FilterPort::Listen()
{
    OVERLAPPED overlapped = { 0 };
    DWORD dwBytesWritten = 0;

    overlapped.hEvent = CreateEvent(nullptr, false, false, nullptr);

    if (!overlapped.hEvent)
    {
        std::wcout << "CreateEvent failed with GLE = " << GetLastError() << std::endl;
        ConsoleAppLogError("CreateEvent failed GLE = 0x%d", GetLastError());
        return;
    }

    while (true)
    {
        unsigned __int8 input[4096] = { 0 };
        auto result = FilterGetMessage(this->driverPort, (PFILTER_MESSAGE_HEADER)(input), sizeof(input), &overlapped);

        if (ERROR_IO_PENDING == (result & 0x7FFF))
        {
            WaitForSingleObject(overlapped.hEvent, INFINITE);
            if (GetOverlappedResult(this->driverPort, &overlapped, &dwBytesWritten, true))
            {
                result = S_OK;
            }
        }

        if (result != S_OK)
        {
            std::wcout << "FilterGetMessage failed with HRESULT = " << result << std::endl;
            ConsoleAppLogError("FilterGetMessage failed with HRESULT = 0x%d", result);
            break;
        }

        Cpp::ShallowStream inputStream{ input, sizeof(input)};
        Cpp::Stream outputStream;

        auto status = HandleMessage(inputStream, outputStream);
        if (status != ERROR_SUCCESS)
        {
            std::wcout << "Can't write filter reply header" << std::endl;
            ConsoleAppLogError("Can't write filter reply header");
            break;
        }

        result = FilterReplyMessage(this->driverPort, (PFILTER_REPLY_HEADER)outputStream.GetRawData(), outputStream.GetSize());
        if (result != S_OK)
        {
            std::wcout << "FilterReplyMessage failed with HRESULT = " << result << std::endl;
            ConsoleAppLogError("FilterReplyMessage failed with HRESULT = 0x%d", result);
            break;
        }
    }

    CloseHandle(overlapped.hEvent);
    this->Disconnect();
}

NTSTATUS
FilterPort::HandleMessage(
    Cpp::ShallowStream& InputData,
    Cpp::Stream& OutputStream
)
{
    FILTER_MESSAGE_HEADER messageHeader = { 0 };
    KmUmShared::FilterMessageHeader filterHeader;
    NTSTATUS status = ERROR_SUCCESS;

    InputData >> messageHeader >> filterHeader;
    if (!InputData.IsValid())
    {
        std::wcout << "Can't read filter message header" << std::endl;
        ConsoleAppLogError("Can't read filter message header");
        return ERROR_INVALID_PARAMETER;
    }

    switch (filterHeader.GetMessageCode())
    {
    case KmUmShared::MessageCode::msgProcessCreate:
        status = HandleMessageNotification<KmUmShared::ProcessCreateMessage>(InputData);
        break;    
    case KmUmShared::MessageCode::msgProcessTerminate:
        status = HandleMessageNotification<KmUmShared::ProcessTerminateMessage>(InputData);
        break;
    case KmUmShared::MessageCode::msgThreadCreate:
        status = HandleMessageNotification<KmUmShared::ThreadCreateMessage>(InputData);
        break;
    case KmUmShared::MessageCode::msgThreadTerminate:
        status = HandleMessageNotification<KmUmShared::ThreadTerminateMessage>(InputData);
        break;
    case KmUmShared::MessageCode::msgModuleLoaded:
        status = HandleMessageNotification<KmUmShared::ModuleMessage>(InputData);
        break;
    case KmUmShared::MessageCode::msgRegistryCreate:
        status = HandleMessageNotification<KmUmShared::RegistryCreateMessage>(InputData);
        break;
    case KmUmShared::MessageCode::msgRegistrySetValue:
        status = HandleMessageNotification<KmUmShared::RegistrySetValueMessage>(InputData);
        break;
    case KmUmShared::MessageCode::msgRegistryDeleteKey:
        status = HandleMessageNotification<KmUmShared::RegistryDeleteKeyMessage>(InputData);
        break;
    case KmUmShared::MessageCode::msgRegistryDeleteValue:
        status = HandleMessageNotification<KmUmShared::RegistryDeleteKeyValueMessage>(InputData);
        break;
    case KmUmShared::MessageCode::msgRegistryRenameKey:
        status = HandleMessageNotification<KmUmShared::RegistryRenameKeyMessage>(InputData);
        break;
    case KmUmShared::MessageCode::msgFileCreate:
        status = HandleMessageNotification<KmUmShared::FileCreateMessage>(InputData);
        break;
    case KmUmShared::MessageCode::msgFileClose:
        status = HandleMessageNotification<KmUmShared::FileCloseMessage>(InputData);
        break;
    case KmUmShared::MessageCode::msgFileCleanup:
        status = HandleMessageNotification<KmUmShared::FileCleanupMessage>(InputData);
        break;
    case KmUmShared::MessageCode::msgFileRead:
        status = HandleMessageNotification<KmUmShared::FileReadMessage>(InputData);
        break;
    case KmUmShared::MessageCode::msgFileWrite:
        status = HandleMessageNotification<KmUmShared::FileWriteMessage>(InputData);
        break;
    case KmUmShared::MessageCode::msgFileSetInformation:
        status = HandleMessageNotification<KmUmShared::FileSetInformationMessage>(InputData);
        break;
    default:
        status = ERROR_NOT_FOUND;
        break;
    }

    FILTER_REPLY_HEADER replyHeader = { 0 };
    replyHeader.MessageId = messageHeader.MessageId;
    replyHeader.Status = ERROR_SUCCESS;

    OutputStream << replyHeader << KmUmShared::FilterReplyHeader{ status };
    if (!OutputStream.IsValid() || OutputStream.GetSize() == 0 || OutputStream.GetSize() > ULONG_MAX)
    {
        std::wcout << "Can't write filter reply header" << std::endl;
        return ERROR_INVALID_MESSAGE;
    }
    return ERROR_SUCCESS;
}

FilterPort::~FilterPort()
{
    CloseConnectionPort();

    listenerThread->join();
    listenerThread.reset(nullptr);
}

NTSTATUS
FilterPort::Send(
    _In_ std::shared_ptr<KmUmShared::CommandHeader> Command,
    _In_ std::shared_ptr<KmUmShared::CommandReply> Reply
)
{
    Cpp::Stream inputStream;
    Cpp::Stream outputStream;

    Command->Serialize(inputStream);
    Reply->Serialize(outputStream);

    if (!inputStream.IsValid() || !outputStream.IsValid())
    {
        std::cout << "Stream is not valid" << std::endl;
        return STATUS_ASSERTION_FAILURE;
    }

    DWORD bytesReturned = 0;
    auto result = FilterSendMessage(
        this->driverPort,
        (LPVOID)inputStream.GetRawData(),
        inputStream.GetSize(),
        (LPVOID)outputStream.GetRawData(),
        outputStream.GetSize(),
        &bytesReturned
    );

    Reply->Deserialize(outputStream);
    if (result != S_OK || !outputStream.IsValid() || bytesReturned != outputStream.GetSize())
    {
        std::wcout << "FilterSendMessage failure" << std::endl;
        return STATUS_ASSERTION_FAILURE;
    }

    return ERROR_SUCCESS;
}
