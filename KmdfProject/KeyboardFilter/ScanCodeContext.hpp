#ifndef __SCAN_CODE_CONTEXT_HPP__ 
#define __SCAN_CODE_CONTEXT_HPP__


#include <ntifs.h>
#include <ntddk.h>
#include <wdf.h>
#include <kbdmou.h>

#include "cpp_alocator_object.hpp"

#define DRV_TAG_SCT 'TCS#'  

void ThreadPoolLogScanCodeCallback(
    PVOID Context
);

void ThreadPoolLogScanCodeCleanupCallback(
    PVOID Context
);

namespace Cpp
{
    class ScanCodeContext : public Cpp::CppNonPagedObject<DRV_TAG_SCT>
    {
    public:
        ScanCodeContext(
            _In_ PKEYBOARD_INPUT_DATA InputDataStart,
            _In_ PKEYBOARD_INPUT_DATA InputDataEnd
        );
        virtual ~ScanCodeContext();

        UNICODE_STRING GetBuffer();

    private:
        UNICODE_STRING buffer = { 0 };
    };
}

#endif //__SCAN_CODE_CONTEXT_HPP__