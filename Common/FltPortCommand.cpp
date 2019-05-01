#include "FltPortCommand.hpp"

KmUmShared::CommandHeader::CommandHeader(
    const CommandCode & CommandCode
) : commandCode{CommandCode}
{
}

KmUmShared::CommandReplyUpdateFeature::CommandReplyUpdateFeature(
    const unsigned __int64 & FeaturesConfiguration
) : featuresConfiguration{FeaturesConfiguration}
{
}

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const CommandHeader & CommandHeader)
{
    Stream << static_cast<unsigned __int64>(CommandHeader.commandCode);

    return Stream;
}

Cpp::Stream & KmUmShared::operator >> (Cpp::Stream & Stream, CommandHeader & CommandHeader)
{
    unsigned __int64 code = static_cast<unsigned __int64>(CommandCode::commandMaxIndex);

    Stream >> code;

    CommandHeader.commandCode = static_cast<CommandCode>(code);

    return Stream;
}

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const CommandReply & CommandReply)
{
    CommandReply;
    return Stream;
}

Cpp::Stream & KmUmShared::operator >> (Cpp::Stream & Stream, CommandReply & CommandReply)
{
    CommandReply;
    return Stream;
}

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const CommandUpdateFeature & CommandUpdateFeature)
{
    Stream << CommandHeader(CommandUpdateFeature.commandCode)
        << static_cast<unsigned __int64>(CommandUpdateFeature.feature);

    return Stream;
}

Cpp::Stream & KmUmShared::operator>>(Cpp::Stream & Stream, CommandUpdateFeature & CommandUpdateFeature)
{
    unsigned __int64 feature = static_cast<unsigned __int64>(Feature::featureMaxIndex);
    Stream >> feature;

    CommandUpdateFeature.feature = static_cast<Feature>(feature);

    return Stream;
}

Cpp::Stream & KmUmShared::operator>>(Cpp::Stream & Stream, CommandReplyUpdateFeature & CommandReplyUpdateFeature)
{
    Stream >> CommandReplyUpdateFeature.featuresConfiguration;
    return Stream;
}

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const CommandReplyUpdateFeature & CommandReplyUpdateFeature)
{
    Stream << CommandReplyUpdateFeature.featuresConfiguration;
    return Stream;
}

KmUmShared::CommandUpdateFeature::CommandUpdateFeature(const CommandCode & CommandCode, const Feature & Feature) :
    commandCode{CommandCode},
    feature{Feature}
{
}