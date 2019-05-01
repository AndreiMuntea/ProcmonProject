#ifndef __MODULE_FILTER_HPP__ 
#define __MODULE_FILTER_HPP__

#include "cpp_allocator_object.hpp"

namespace Minifilter
{
    class ModuleFilter : public Cpp::CppNonPagedObject<'TFM#'>
    {
    public:
        ModuleFilter();
        virtual ~ModuleFilter();

        static void
        LoadImageNotifyRoutine(
            _In_opt_ PUNICODE_STRING FullImageName,
            _In_ HANDLE ProcessId,
            _In_ PIMAGE_INFO ImageInfo
        );

    private:
        static bool
        IsActionMonitored(
            _In_ HANDLE ProcessId
        );
    };
}

#endif //__MODULE_FILTER_HPP__