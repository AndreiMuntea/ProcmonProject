#ifndef __FILE_HPP__ 
#define __FILE_HPP__

#include <ntifs.h>
#include <ntddk.h>
#include <wdf.h>

#include "cpp_alocator_object.hpp"

#define DRV_TAG_KFT 'TKF#'  

namespace Cpp
{
    class File : public Cpp::CppNonPagedObject<DRV_TAG_KFT>
    {
    public:
        File(_In_ UNICODE_STRING Path);
        virtual ~File();

        NTSTATUS Write(_In_ UNICODE_STRING Buffer);

    private:
        HANDLE handle = nullptr;
    };
}
#endif //__FILE_HPP__