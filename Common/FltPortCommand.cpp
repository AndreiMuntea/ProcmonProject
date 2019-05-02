#include "FltPortCommand.hpp"

KmUmShared::CommandHeader::CommandHeader(
    const CommandCode & CommandCode
) : commandCode{CommandCode}
{
}

void KmUmShared::CommandHeader::Serialize(Cpp::Stream & Stream) const
{
    Stream << commandCode;
}

void KmUmShared::CommandHeader::Deserialize(Cpp::Stream & Stream)
{
    Stream >> commandCode;
}

KmUmShared::CommandReplyUpdateFeature::CommandReplyUpdateFeature(
    const unsigned __int64 & FeaturesConfiguration
) : featuresConfiguration{FeaturesConfiguration}
{
}

void KmUmShared::CommandReplyUpdateFeature::Serialize(Cpp::Stream & Stream) const
{
    Stream << this->featuresConfiguration;
}

void KmUmShared::CommandReplyUpdateFeature::Deserialize(Cpp::Stream & Stream)
{
    Stream >> this->featuresConfiguration;
}

KmUmShared::CommandUpdateFeature::CommandUpdateFeature(
    const CommandCode & CommandCode, 
    const Feature & Feature
) : CommandHeader{CommandCode},
    feature{Feature}
{
}

void KmUmShared::CommandUpdateFeature::Serialize(Cpp::Stream & Stream) const
{
    CommandHeader::Serialize(Stream);
    Stream << this->feature;
}

void KmUmShared::CommandUpdateFeature::Deserialize(Cpp::Stream & Stream)
{
    Stream >> this->feature;
}

void KmUmShared::CommandReply::Serialize(Cpp::Stream & Stream) const
{
    Stream << this->dummy;
}

void KmUmShared::CommandReply::Deserialize(Cpp::Stream & Stream)
{
    Stream >> this->dummy;
}

KmUmShared::CommandUpdateBlacklistFolder::CommandUpdateBlacklistFolder(
    const CommandCode & CommandCode, 
    const Cpp::String & Folder
) : CommandHeader{ CommandCode },
    folder{Folder}
{
}

void KmUmShared::CommandUpdateBlacklistFolder::Serialize(Cpp::Stream & Stream) const
{
    CommandHeader::Serialize(Stream);
    Stream << this->folder;
}

void KmUmShared::CommandUpdateBlacklistFolder::Deserialize(Cpp::Stream & Stream)
{
    Stream >> this->folder;
}
