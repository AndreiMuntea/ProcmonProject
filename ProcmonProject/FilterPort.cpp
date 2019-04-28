#include "FilterPort.hpp"
#include "trace.h"
#include "FilterPort.tmh"

#include <iostream>


FilterPort::FilterPort(const std::wstring & PortName) 
    : portName{PortName}
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

void FilterPort::CloseConnectionPort()
{
    if (INVALID_HANDLE_VALUE != driverPort)
    {
        CloseHandle(driverPort);
        driverPort = INVALID_HANDLE_VALUE;
    }
}

void FilterPort::Listen()
{
    while (true)
    {
        unsigned __int8 input[4096] = { 0 };
        auto result = FilterGetMessage(this->driverPort, (PFILTER_MESSAGE_HEADER)(input), sizeof(input), nullptr);
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