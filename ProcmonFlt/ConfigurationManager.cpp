#include "ConfigurationManager.hpp"

#include <intrin.h>

Minifilter::ConfigurationManager::ConfigurationManager()
{
    Validate();
}

NTSTATUS
Minifilter::ConfigurationManager::EnableFeature(
    const KmUmShared::Feature & Feature
)
{
    if (Feature >= KmUmShared::Feature::featureMaxIndex)
    {
        return STATUS_DEVICE_FEATURE_NOT_SUPPORTED;
    }
    
    InterlockedOr64(&features, GetFeatureIndex(Feature));
    return STATUS_SUCCESS;
}

NTSTATUS 
Minifilter::ConfigurationManager::DisableFeature(
    const KmUmShared::Feature & Feature
)
{
    if (Feature >= KmUmShared::Feature::featureMaxIndex)
    {
        return STATUS_DEVICE_FEATURE_NOT_SUPPORTED;
    }

    InterlockedAnd64(&features, ~ GetFeatureIndex(Feature));
    return STATUS_SUCCESS;
}

bool 
Minifilter::ConfigurationManager::IsFeatureEnabled(
    const KmUmShared::Feature & Feature
) const
{
    if (Feature >= KmUmShared::Feature::featureMaxIndex)
    {
        return false;
    }

    return BooleanFlagOn(features, GetFeatureIndex(Feature));
}

LONG64 
Minifilter::ConfigurationManager::GetCurrentConfiguration() const
{
    return features;
}

NTSTATUS Minifilter::ConfigurationManager::SetConfiguration(LONG64 Configuration)
{
    unsigned long index = 0;

    if (_BitScanReverse64(&index, Configuration))
    {
        if (static_cast<KmUmShared::Feature>(index) >= KmUmShared::Feature::featureMaxIndex)
        {
            return STATUS_INVALID_PARAMETER;
        }
    }

    InterlockedExchange64(&this->features, Configuration);
    return STATUS_SUCCESS;
}

LONG64 
Minifilter::ConfigurationManager::GetFeatureIndex(
    const KmUmShared::Feature & Feature
) const
{
    auto index = static_cast<LONG64>(Feature);
    return static_cast<LONG64>(1) << index;
}
