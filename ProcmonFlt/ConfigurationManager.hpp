#ifndef __CONFIGURATION_MANAGER_HPP__ 
#define __CONFIGURATION_MANAGER_HPP__

#include "cpp_allocator_object.hpp"
#include "../Common/FltPortCommand.hpp"

namespace Minifilter
{


    class ConfigurationManager : public Cpp::CppNonPagedObject<'GMC#'>
    {
    public:
        ConfigurationManager() = default;
        virtual ~ConfigurationManager() = default;

        NTSTATUS EnableFeature(const KmUmShared::Feature& Feature);
        NTSTATUS DisableFeature(const KmUmShared::Feature& Feature);
        bool IsFeatureEnabled(const KmUmShared::Feature& Feature) const;
        LONG64 GetCurrentConfiguration() const;

    private:
        LONG64 GetFeatureIndex(const KmUmShared::Feature& Feature) const;

        volatile LONG64 features = 0;
    };
}

#endif //__CONFIGURATION_MANAGER_HPP__