#ifndef __FLT_PORT_COMMAND_HPP__
#define __FLT_PORT_COMMAND_HPP__

#include <CppStream.hpp>

namespace KmUmShared
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

    enum class CommandCode
    {
        commandEnableFeature = 0,
        commandDisableFeature,
        commandMaxIndex
    };

    class CommandHeader
    {
    public:
        CommandHeader() = default;
        CommandHeader(const CommandCode& CommandCode);
        virtual ~CommandHeader() = default;

        virtual void Serialize(Cpp::Stream& Stream) const;
        virtual void Deserialize(Cpp::Stream& Stream);

    public:
        CommandCode commandCode = CommandCode::commandMaxIndex;
    };

    class CommandReply
    {
    public:
        CommandReply() = default;
        virtual ~CommandReply() = default;

        virtual void Serialize(Cpp::Stream& Stream) const;
        virtual void Deserialize(Cpp::Stream& Stream);

    public:
    };

    class CommandUpdateFeature : public CommandHeader
    {
    public:
        CommandUpdateFeature() = default;
        CommandUpdateFeature(const CommandCode& CommandCode, const Feature& Feature);
        virtual ~CommandUpdateFeature() = default;

        virtual void Serialize(Cpp::Stream& Stream) const override;
        virtual void Deserialize(Cpp::Stream& Stream) override;

    public:
        Feature feature = Feature::featureMaxIndex;
    };

    class CommandReplyUpdateFeature : public CommandReply
    {
    public:
        CommandReplyUpdateFeature() = default;
        CommandReplyUpdateFeature(const unsigned __int64& FeaturesConfiguration);
        virtual ~CommandReplyUpdateFeature() = default;

        virtual void Serialize(Cpp::Stream& Stream) const override;
        virtual void Deserialize(Cpp::Stream& Stream) override;

    public:
        unsigned __int64 featuresConfiguration = 0;
    };

}

#endif //__FLT_PORT_COMMAND_HPP__