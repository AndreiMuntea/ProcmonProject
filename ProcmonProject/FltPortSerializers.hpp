#ifndef __FLT_PORT_SERIALIZERS_HPP__ 
#define __FLT_PORT_SERIALIZERS_HPP__

#include "../Common/FltPortMessage.hpp"
#include "../Common/FltPortProcessMessage.hpp"
#include "../Common/FltPortThreadMessage.hpp"
#include "../Common/FltPortModuleMessage.hpp"
#include "../Common/FltPortRegistryMessage.hpp"
#include "../Common/FltPortFileMessage.hpp"
#include "../Common/FltPortNetworkMessage.hpp"

#include <CppString.hpp>
#include <CppNonPagedString.hpp>
#include <sstream>
#include <string>

std::wostream& operator<<(std::wostream& Stream, Cpp::String& String);
std::wostream& operator<<(std::wostream& Stream, Cpp::NonPagedString& NonPagedString);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::ProcessCreateMessage& ProcessCreateMessage);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::ProcessTerminateMessage& ProcessTerminateMessage);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::ThreadCreateMessage& ThreadCreateMessage);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::ThreadTerminateMessage& ThreadTerminateMessage);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::ModuleMessage& ModuleMessage);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::RegistryCreateMessage& RegistryCreateMessage);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::RegistrySetValueMessage& RegistrySetValueMessage);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::RegistryDeleteKeyValueMessage& RegistryDeleteKeyValueMessage);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::RegistryDeleteKeyMessage& RegistryDeleteKeyMessage);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::RegistryLoadKeyMessage& RegistryLoadKeyMessage);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::RegistryRenameKeyMessage& RegistryLoadKeyMessage);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::FileCreateMessage& FileCreateMessage);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::FileCloseMessage& FileCloseMessage);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::FileCleanupMessage& FileCleanupMessage);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::FileReadMessage& FileReadMessage);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::FileWriteMessage& FileWriteMessage);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::FileSetInformationMessage& FileSetInformationMessage);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::FileDeleteMessage& FileDeleteMessage);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::NetworkMessageIpV4& NetworkMessageIpV4);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::NetworkMessageIpV6& NetworkMessageIpV6);

#endif //__FLT_PORT_SERIALIZERS_HPP__