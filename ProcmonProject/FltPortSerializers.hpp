#ifndef __FLT_PORT_SERIALIZERS_HPP__ 
#define __FLT_PORT_SERIALIZERS_HPP__

#include "../Common/FltPortMessage.hpp"
#include "../Common/FltPortProcessCreateMessage.hpp"
#include "../Common/FltPortProcessTerminateMessage.hpp"

#include <CppString.hpp>
#include <sstream>
#include <string>

std::wostream& operator<<(std::wostream& Stream, Cpp::String& String);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::ProcessCreateMessage& ProcessCreateMessage);
std::wostream& operator<<(std::wostream& Stream, KmUmShared::ProcessTerminateMessage& ProcessTerminateMessage);

#endif //__FLT_PORT_SERIALIZERS_HPP__