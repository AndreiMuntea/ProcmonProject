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


        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const CommandHeader& CommandHeader);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, CommandHeader& CommandHeader);
    
    public:
        CommandCode commandCode = CommandCode::commandMaxIndex;
    };

    class CommandReply
    {
    public:
        CommandReply() = default;
        virtual ~CommandReply() = default;


        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const CommandReply& CommandReply);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, CommandReply& CommandReply);

    public:
    };

    class CommandUpdateFeature : public CommandHeader
    {
    public:
        CommandUpdateFeature() = default;
        CommandUpdateFeature(const CommandCode& CommandCode, const Feature& Feature);
        virtual ~CommandUpdateFeature() = default;


        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const CommandUpdateFeature& CommandUpdateFeature);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, CommandUpdateFeature& CommandUpdateFeature);

    public:
        CommandCode commandCode = CommandCode::commandMaxIndex;
        Feature feature = Feature::featureMaxIndex;
    };

    class CommandReplyUpdateFeature : public CommandReply
    {
    public:
        CommandReplyUpdateFeature() = default;
        CommandReplyUpdateFeature(const unsigned __int64& FeaturesConfiguration);
        virtual ~CommandReplyUpdateFeature() = default;

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const CommandReplyUpdateFeature& CommandReplyUpdateFeature);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, CommandReplyUpdateFeature& CommandReplyUpdateFeature);

    public:
        unsigned __int64 featuresConfiguration = 0;
    };

}

#endif //__FLT_PORT_COMMAND_HPP__