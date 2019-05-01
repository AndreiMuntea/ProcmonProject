#include "ConfigurationManager.hpp"

NTSTATUS 
Minifilter::ConfigurationManager::EnableFeature(
    const Feature & Feature
)
{
    if (Feature >= Feature::featureMaxIndex)
    {
        return STATUS_DEVICE_FEATURE_NOT_SUPPORTED;
    }
    
    InterlockedOr64(&features, GetFeatureIndex(Feature));
    return STATUS_SUCCESS;
}

NTSTATUS 
Minifilter::ConfigurationManager::DisableFeature(
    const Feature & Feature
)
{
    if (Feature >= Feature::featureMaxIndex)
    {
        return STATUS_DEVICE_FEATURE_NOT_SUPPORTED;
    }

    InterlockedAnd64(&features, ~ GetFeatureIndex(Feature));
    return STATUS_SUCCESS;
}

bool 
Minifilter::ConfigurationManager::IsFeatureEnabled(
    const Feature & Feature
) const
{
    if (Feature >= Feature::featureMaxIndex)
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

LONG64 
Minifilter::ConfigurationManager::GetFeatureIndex(
    const Feature & Feature
) const
{
    auto index = static_cast<LONG64>(Feature);
    return static_cast<LONG64>(1) << index;
}
