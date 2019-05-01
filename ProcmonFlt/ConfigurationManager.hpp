#ifndef __CONFIGURATION_MANAGER_HPP__ 
#define __CONFIGURATION_MANAGER_HPP__

#include "cpp_allocator_object.hpp"

namespace Minifilter
{
    enum class Feature
    {
        featureMonitorStarted = 0,
        featureMonitorProcessCreate,
        featureMonitorProcessTerminate,
        featureMonitorThreadCreate,
        featureMonitorThreadTerminate,
        featureMonitorImageLoaded,
        featureMonitorRegistryCreateKey,
        featureMonitorRegistrySetValue,
        featureMonitorRegistryDeleteKey,
        featureMonitorRegistryDeleteKeyValue,
        featureMonitorRegistryLoadKey,
        featureMonitorRegistryRenameKey,
        featureMonitorFileCreate,
        featureMonitorFileClose,
        featureMonitorFileCleanup,
        featureMonitorFileRead,
        featureMonitorFileWrite,
        featureMonitorFileSetInformation,
        featureMaxIndex
    };

    class ConfigurationManager : public Cpp::CppNonPagedObject<'GMC#'>
    {
    public:
        ConfigurationManager() = default;
        virtual ~ConfigurationManager() = default;

        NTSTATUS EnableFeature(const Feature& Feature);
        NTSTATUS DisableFeature(const Feature& Feature);
        bool IsFeatureEnabled(const Feature& Feature) const;
        LONG64 GetCurrentConfiguration() const;

    private:
        LONG64 GetFeatureIndex(const Feature& Feature) const;

        volatile LONG64 features = 0;
    };
}

#endif //__CONFIGURATION_MANAGER_HPP__