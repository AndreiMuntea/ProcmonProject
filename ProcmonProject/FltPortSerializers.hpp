#ifndef __FLT_PORT_SERIALIZERS_HPP__ 
#define __FLT_PORT_SERIALIZERS_HPP__

#include "../Common/FltPortMessage.hpp"
#include "../Common/FltPortProcessMessage.hpp"
#include "../Common/FltPortThreadMessage.hpp"
#include "../Common/FltPortModuleMessage.hpp"

#include <CppString.hpp>
#include <sstream>
#include <string>

std::wostream& operator<<(std::wostream& Stream, Cpp::String& String);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::ProcessCreateMessage& ProcessCreateMessage);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::ProcessTerminateMessage& ProcessTerminateMessage);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::ThreadCreateMessage& ThreadCreateMessage);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::ThreadTerminateMessage& ThreadTerminateMessage);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::ModuleMessage& ModuleMessage);

#endif //__FLT_PORT_SERIALIZERS_HPP__