#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "FltPortSerializers.hpp"
#include <WinSock2.h>

std::wostream& operator<<(std::wostream& Stream, Cpp::String& String)
{
    Stream.write((const wchar_t*)String.GetNakedPointer(), String.GetSize() / sizeof(wchar_t));
    return Stream;
}

std::wostream& operator<<(std::wostream& Stream, KmUmShared::FileDeleteType DeleteType)
{
    switch (DeleteType)
    {
    case KmUmShared::FileDeleteType::DeleteOnClose:
        Stream << "DeleteOnClose";
        return Stream;
    case KmUmShared::FileDeleteType::PendingRegistry:
        Stream << "PendingRegistry";
        return Stream;
    case KmUmShared::FileDeleteType::SetFileInformation:
        Stream << "SetFileInformation";
        return Stream;
    case KmUmShared::FileDeleteType::FileOverwritten:
        Stream << "Overwritten";
        return Stream;
    default:
        return Stream;
    }
}

std::wostream& operator<<(std::wostream& Stream, KmUmShared::ProcessCreateMessage& ProcessCreateMessage)
{
    Stream << "[ProcessCreateMessage]" << std::endl
        << "\t> [Command Line] " << ProcessCreateMessage.commandLine << std::endl;

    return Stream;
}

std::wostream& operator<<(std::wostream& Stream, KmUmShared::ProcessTerminateMessage& ProcessTerminateMessage)
{
    ProcessTerminateMessage;

    Stream << "[ProcessTerminateMessage]" << std::endl;
    return Stream;
}

std::wostream & operator<<(std::wostream & Stream, KmUmShared::ThreadCreateMessage & ThreadCreateMessage)
{
    Stream << "[ThreadCreateMessage]" << std::endl
        << "\t> [Thread ID] " << ThreadCreateMessage.threadId << std::endl;

    return Stream;
}

std::wostream & operator<<(std::wostream & Stream, KmUmShared::ThreadTerminateMessage & ThreadTerminateMessage)
{
    Stream << "[ThreadTerminateMessage]" << std::endl
        << "\t> [Thread ID] " << ThreadTerminateMessage.threadId << std::endl;

    return Stream;
}

std::wostream & operator<<(std::wostream & Stream, KmUmShared::ModuleMessage & ModuleMessage)
{
    Stream << "[ImageLoadedMessage]" << std::endl
        << "\t> [Image Name] " << ModuleMessage.imageName << std::endl
        << "\t> [Image Base] " << std::hex << ModuleMessage.imageBase << std::dec << std::endl
        << "\t> [Image Size] " << std::hex << ModuleMessage.imageSize << std::dec << std::endl;

    return Stream;
}

std::wostream & operator<<(std::wostream & Stream, KmUmShared::RegistryCreateMessage & RegistryCreateMessage)
{
    Stream << "[RegistryCreateMessage]" << std::endl
        << "\t> [Key Name] " << RegistryCreateMessage .string1 << std::endl
        << "\t> [Status] " << RegistryCreateMessage.status << std::endl;

    return Stream;
}

std::wostream & operator<<(std::wostream & Stream, KmUmShared::RegistrySetValueMessage & RegistrySetValueMessage)
{
    Stream << "[RegistrySetValueMessage]" << std::endl
        << "\t> [Key Name] " << RegistrySetValueMessage.string1 << std::endl
        << "\t> [Value Name] " << RegistrySetValueMessage.string2 << std::endl
        << "\t> [Data] " << RegistrySetValueMessage.string3<< std::endl
        << "\t> [Data Type] " << RegistrySetValueMessage.dataType << std::endl
        << "\t> [Status] " << RegistrySetValueMessage.status << std::endl;

    return Stream;
}


std::wostream& operator<<(std::wostream& Stream, KmUmShared::RegistryDeleteKeyValueMessage& RegistryDeleteKeyValueMessage)
{
    Stream << "[RegistryDeleteKeyValueMessage]" << std::endl
        << "\t> [Key Name] " << RegistryDeleteKeyValueMessage.string1 << std::endl
        << "\t> [Value Name] " << RegistryDeleteKeyValueMessage.string2 << std::endl
        << "\t> [Status] " << RegistryDeleteKeyValueMessage.status << std::endl;

    return Stream;
}

std::wostream& operator<<(std::wostream& Stream, KmUmShared::RegistryDeleteKeyMessage& RegistryDeleteKeyMessage)
{
    Stream << "[RegistryDeleteMessage]" << std::endl
        << "\t> [Key Name] " << RegistryDeleteKeyMessage.string1 << std::endl
        << "\t> [Status] " << RegistryDeleteKeyMessage.status << std::endl;

    return Stream;
}

std::wostream& operator<<(std::wostream& Stream, KmUmShared::RegistryLoadKeyMessage& RegistryLoadKeyMessage)
{
    Stream << "[RegistryLoadKeyMessage]" << std::endl
        << "\t> [Key Name] " << RegistryLoadKeyMessage.string1 << std::endl
        << "\t> [Status] " << RegistryLoadKeyMessage.status << std::endl;

    return Stream;
}

std::wostream & operator<<(std::wostream & Stream, KmUmShared::RegistryRenameKeyMessage & RegistryLoadKeyMessage)
{
    Stream << "[RegistryRenameKeyMessage]" << std::endl
        << "\t> [Old Key Name] " << RegistryLoadKeyMessage.string1 << std::endl
        << "\t> [New Key Name] " << RegistryLoadKeyMessage.string2 << std::endl
        << "\t> [Status] " << RegistryLoadKeyMessage.status << std::endl;

    return Stream;
}

std::wostream& operator<<(std::wostream& Stream, KmUmShared::FileCreateMessage& FileCreateMessage)
{
    Stream << "[FileCreateMessage]" << std::endl
        << "\t> [File Name] " << FileCreateMessage.string1 << std::endl
        << "\t> [Status] " << FileCreateMessage.status << std::endl;
    return Stream;
}

std::wostream & operator<<(std::wostream & Stream, KmUmShared::FileCloseMessage & FileCloseMessage)
{
    Stream << "[FileCloseMessage]" << std::endl
        << "\t> [File Name] " << FileCloseMessage.string1 << std::endl
        << "\t> [Status] " << FileCloseMessage.status << std::endl;
    return Stream;
}

std::wostream & operator<<(std::wostream & Stream, KmUmShared::FileCleanupMessage & FileCleanupMessage)
{
    Stream << "[FileCleanupMessage]" << std::endl
        << "\t> [File Name] " << FileCleanupMessage.string1 << std::endl
        << "\t> [Status] " << FileCleanupMessage.status << std::endl;
    return Stream;
}

std::wostream & operator<<(std::wostream & Stream, KmUmShared::FileReadMessage & FileReadMessage)
{
    Stream << "[FileReadMessage]" << std::endl
        << "\t> [File Name] " << FileReadMessage.string1 << std::endl
        << "\t> [Status] " << FileReadMessage.status << std::endl;
    return Stream;
}

std::wostream & operator<<(std::wostream & Stream, KmUmShared::FileWriteMessage & FileWriteMessage)
{
    Stream << "[FileWriteMessage]" << std::endl
        << "\t> [File Name] " << FileWriteMessage.string1 << std::endl
        << "\t> [Status] " << FileWriteMessage.status << std::endl;
    return Stream;
}


std::wostream& operator<<(std::wostream& Stream, KmUmShared::FileSetInformationMessage& FileSetInformationMessage)
{
    Stream << "[FileSetInformationMessage]" << std::endl
        << "\t> [File Name] " << FileSetInformationMessage.string1 << std::endl
        << "\t> [Status] " << FileSetInformationMessage.status << std::endl;
    return Stream;
}

std::wostream & operator<<(std::wostream & Stream, KmUmShared::FileDeleteMessage & FileDeleteMessage)
{
    Stream << "[FileDeleteMessage]" << std::endl
        << "\t> [File Name] " << FileDeleteMessage.string1 << std::endl
        << "\t> [Status] " << FileDeleteMessage.status << std::endl
        << "\t> [DeleteType] " << FileDeleteMessage.deleteType << std::endl;
    return Stream;
}

std::wostream & operator<<(std::wostream & Stream, KmUmShared::NetworkMessage & NetworkMessage)
{
    Stream << "[Network activity]" << std::endl
        << "\t> [ApplicationId ] " << NetworkMessage.applicationId << std::endl
        << "\t> [ICMP] " << NetworkMessage.icmp << std::endl
        << "\t> [Local Address] " << inet_ntoa(*(struct in_addr*)(&NetworkMessage.localAddress))  << std::endl
        << "\t> [Remote Address] " << inet_ntoa(*(struct in_addr*)(&NetworkMessage.remoteAddress)) << std::endl
        << "\t> [Local Port] " << NetworkMessage.localPort << std::endl
        << "\t> [Remote Port] " << NetworkMessage.remoteAddress << std::endl;
    return Stream;
}
