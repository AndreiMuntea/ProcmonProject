#include "FltPortSerializers.hpp"

std::wostream& operator<<(std::wostream& Stream, Cpp::String& String)
{
    Stream.write((const wchar_t*)String.GetNakedPointer(), String.GetSize() / sizeof(wchar_t));
    return Stream;
}

std::wostream& operator<<(std::wostream& Stream, KmUmShared::ProcessCreateMessage& ProcessCreateMessage)
{
    Stream << "[ProcessCreateMessage]" << std::endl
        << "\t> [Timestamp] " << ProcessCreateMessage.timestamp << std::endl
        << "\t> [Parent ID] " << ProcessCreateMessage.parentPid << std::endl
        << "\t> [Process ID] " << ProcessCreateMessage.processId << std::endl
        << "\t> [Image Path] " << ProcessCreateMessage.imagePath << std::endl
        << "\t> [Command Line] " << ProcessCreateMessage.commandLine << std::endl
        << std::endl;

    return Stream;
}

std::wostream& operator<<(std::wostream& Stream, KmUmShared::ProcessTerminateMessage& ProcessTerminateMessage)
{
    Stream << "[ProcessTerminateMessage]" << std::endl
        << "\t> [Timestamp] " << ProcessTerminateMessage.timestamp << std::endl
        << "\t> [Process ID] " << ProcessTerminateMessage.processId << std::endl
        << std::endl;

    return Stream;
}

std::wostream & operator<<(std::wostream & Stream, KmUmShared::ThreadCreateMessage & ThreadCreateMessage)
{
    Stream << "[ThreadCreateMessage]" << std::endl
        << "\t> [Timestamp] " << ThreadCreateMessage.timestamp << std::endl
        << "\t> [Process ID] " << ThreadCreateMessage.processId << std::endl
        << "\t> [Thread ID] " << ThreadCreateMessage.threadId << std::endl
        << std::endl;

    return Stream;
}

std::wostream & operator<<(std::wostream & Stream, KmUmShared::ThreadTerminateMessage & ThreadTerminateMessage)
{
    Stream << "[ThreadTerminateMessage]" << std::endl
        << "\t> [Timestamp] " << ThreadTerminateMessage.timestamp << std::endl
        << "\t> [Process ID] " << ThreadTerminateMessage.processId << std::endl
        << "\t> [Thread ID] " << ThreadTerminateMessage.threadId << std::endl
        << std::endl;

    return Stream;
}

std::wostream & operator<<(std::wostream & Stream, KmUmShared::ModuleMessage & ModuleMessage)
{
    Stream << "[ImageLoadedMessage]" << std::endl
        << "\t> [Timestamp] " << ModuleMessage.timestamp << std::endl
        << "\t> [Process ID] " << ModuleMessage.processId << std::endl
        << "\t> [Image Name] " << ModuleMessage.imageName << std::endl
        << "\t> [Image Base] " << std::hex << ModuleMessage.imageBase << std::dec << std::endl
        << "\t> [Image Size] " << std::hex << ModuleMessage.imageSize << std::dec << std::endl
        << std::endl;

    return Stream;
}

std::wostream & operator<<(std::wostream & Stream, KmUmShared::RegistryCreateMessage & RegistryCreateMessage)
{
    Stream << "[RegistryCreateMessage]" << std::endl
        << "\t> [Timestamp] " << RegistryCreateMessage.timestamp << std::endl
        << "\t> [Process ID] " << RegistryCreateMessage.processId << std::endl
        << "\t> [Key Name] " << RegistryCreateMessage .string1 << std::endl
        << "\t> [Status] " << RegistryCreateMessage.status << std::endl
        << std::endl;

    return Stream;
}

std::wostream & operator<<(std::wostream & Stream, KmUmShared::RegistrySetValueMessage & RegistrySetValueMessage)
{
    Stream << "[RegistrySetValueMessage]" << std::endl
        << "\t> [Timestamp] " << RegistrySetValueMessage.timestamp << std::endl
        << "\t> [Process ID] " << RegistrySetValueMessage.processId << std::endl
        << "\t> [Key Name] " << RegistrySetValueMessage.string1 << std::endl
        << "\t> [Value Name] " << RegistrySetValueMessage.string2 << std::endl
        << "\t> [Status] " << RegistrySetValueMessage.status << std::endl
        << std::endl;

    return Stream;
}


std::wostream& operator<<(std::wostream& Stream, KmUmShared::RegistryDeleteKeyValueMessage& RegistryDeleteKeyValueMessage)
{
    Stream << "[RegistryDeleteKeyValueMessage]" << std::endl
        << "\t> [Timestamp] " << RegistryDeleteKeyValueMessage.timestamp << std::endl
        << "\t> [Process ID] " << RegistryDeleteKeyValueMessage.processId << std::endl
        << "\t> [Key Name] " << RegistryDeleteKeyValueMessage.string1 << std::endl
        << "\t> [Value Name] " << RegistryDeleteKeyValueMessage.string2 << std::endl
        << "\t> [Status] " << RegistryDeleteKeyValueMessage.status << std::endl
        << std::endl;

    return Stream;
}

std::wostream& operator<<(std::wostream& Stream, KmUmShared::RegistryDeleteKeyMessage& RegistryDeleteKeyMessage)
{
    Stream << "[RegistryDeleteMessage]" << std::endl
        << "\t> [Timestamp] " << RegistryDeleteKeyMessage.timestamp << std::endl
        << "\t> [Process ID] " << RegistryDeleteKeyMessage.processId << std::endl
        << "\t> [Key Name] " << RegistryDeleteKeyMessage.string1 << std::endl
        << "\t> [Status] " << RegistryDeleteKeyMessage.status << std::endl
        << std::endl;

    return Stream;
}

std::wostream& operator<<(std::wostream& Stream, KmUmShared::RegistryLoadKeyMessage& RegistryLoadKeyMessage)
{
    Stream << "[RegistryLoadKeyMessage]" << std::endl
        << "\t> [Timestamp] " << RegistryLoadKeyMessage.timestamp << std::endl
        << "\t> [Process ID] " << RegistryLoadKeyMessage.processId << std::endl
        << "\t> [Key Name] " << RegistryLoadKeyMessage.string1 << std::endl
        << "\t> [Status] " << RegistryLoadKeyMessage.status << std::endl
        << std::endl;

    return Stream;
}

std::wostream & operator<<(std::wostream & Stream, KmUmShared::RegistryRenameKeyMessage & RegistryLoadKeyMessage)
{
    Stream << "[RegistryDeleteKeyValueMessage]" << std::endl
        << "\t> [Timestamp] " << RegistryLoadKeyMessage.timestamp << std::endl
        << "\t> [Process ID] " << RegistryLoadKeyMessage.processId << std::endl
        << "\t> [Old Key Name] " << RegistryLoadKeyMessage.string1 << std::endl
        << "\t> [New Key Name] " << RegistryLoadKeyMessage.string2 << std::endl
        << "\t> [Status] " << RegistryLoadKeyMessage.status << std::endl
        << std::endl;
    return Stream;
}

std::wostream& operator<<(std::wostream& Stream, KmUmShared::FileCreateMessage& FileCreateMessage)
{
    Stream << "[FileCreateMessage]" << std::endl
        << "\t> [Timestamp] " << FileCreateMessage.timestamp << std::endl
        << "\t> [Process ID] " << FileCreateMessage.processId << std::endl
        << "\t> [File Name] " << FileCreateMessage.string1 << std::endl
        << "\t> [Status] " << FileCreateMessage.status << std::endl
        << std::endl;
    return Stream;
}

std::wostream & operator<<(std::wostream & Stream, KmUmShared::FileCloseMessage & FileCloseMessage)
{
    Stream << "[FileCloseMessage]" << std::endl
        << "\t> [Timestamp] " << FileCloseMessage.timestamp << std::endl
        << "\t> [Process ID] " << FileCloseMessage.processId << std::endl
        << "\t> [File Name] " << FileCloseMessage.string1 << std::endl
        << "\t> [Status] " << FileCloseMessage.status << std::endl
        << std::endl;
    return Stream;
}

std::wostream & operator<<(std::wostream & Stream, KmUmShared::FileCleanupMessage & FileCleanupMessage)
{
    Stream << "[FileCleanupMessage]" << std::endl
        << "\t> [Timestamp] " << FileCleanupMessage.timestamp << std::endl
        << "\t> [Process ID] " << FileCleanupMessage.processId << std::endl
        << "\t> [File Name] " << FileCleanupMessage.string1 << std::endl
        << "\t> [Status] " << FileCleanupMessage.status << std::endl
        << std::endl;
    return Stream;
}

std::wostream & operator<<(std::wostream & Stream, KmUmShared::FileReadMessage & FileReadMessage)
{
    Stream << "[FileReadMessage]" << std::endl
        << "\t> [Timestamp] " << FileReadMessage.timestamp << std::endl
        << "\t> [Process ID] " << FileReadMessage.processId << std::endl
        << "\t> [File Name] " << FileReadMessage.string1 << std::endl
        << "\t> [Status] " << FileReadMessage.status << std::endl
        << std::endl;
    return Stream;
}

std::wostream & operator<<(std::wostream & Stream, KmUmShared::FileWriteMessage & FileWriteMessage)
{
    Stream << "[FileWriteMessage]" << std::endl
        << "\t> [Timestamp] " << FileWriteMessage.timestamp << std::endl
        << "\t> [Process ID] " << FileWriteMessage.processId << std::endl
        << "\t> [File Name] " << FileWriteMessage.string1 << std::endl
        << "\t> [Status] " << FileWriteMessage.status << std::endl
        << std::endl;
    return Stream;
}
