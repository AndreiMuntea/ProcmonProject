#ifndef __COMMANDS_HPP__ 
#define __COMMANDS_HPP__

#include <fltKernel.h>
#include <CppShallowStream.hpp>
#include "../Common/FltPortCommand.hpp"
#include "GlobalData.hpp"

NTSTATUS
CommandOnUpdateFeature(
    _In_ bool Enable,
    _Inout_ Cpp::ShallowStream& InputStream,
    _Inout_ Cpp::Stream& OutputStream
);

NTSTATUS
CommandOnUpdateBlacklistedPath(
    _In_ bool Blacklist,
    _Inout_ Cpp::ShallowStream& InputStream,
    _Inout_ Cpp::Stream& OutputStream
);

NTSTATUS
CommandOnSetConfiguration(
    _Inout_ Cpp::ShallowStream& InputStream,
    _Inout_ Cpp::Stream& OutputStream
);

#endif //__COMMANDS_HPP__