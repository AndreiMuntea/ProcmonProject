#include "Commands.hpp"

NTSTATUS 
CommandOnUpdateFeature(
    _In_ bool Enable,
    _Inout_ Cpp::ShallowStream& InputStream,
    _Inout_ Cpp::Stream& OutputStream
)
{
    KmUmShared::CommandUpdateFeature command;
    KmUmShared::CommandReplyUpdateFeature reply;

    command.Deserialize(InputStream);
    if (!InputStream.IsValid() || command.feature >= KmUmShared::Feature::featureMaxIndex)
    {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    auto status = (Enable) ? gDrvData.ConfigurationManager->EnableFeature(command.feature)
                           : gDrvData.ConfigurationManager->DisableFeature(command.feature);

    reply.featuresConfiguration = gDrvData.ConfigurationManager->GetCurrentConfiguration();
    reply.Serialize(OutputStream);

    return status;
}

NTSTATUS 
CommandOnUpdateBlacklistedPath(
    _In_ bool Blacklist,
    _Inout_ Cpp::ShallowStream& InputStream,
    _Inout_ Cpp::Stream& OutputStream
)
{
    UNREFERENCED_PARAMETER(OutputStream);
    KmUmShared::CommandUpdateBlacklistFolder command;

    command.Deserialize(InputStream);
    if (!InputStream.IsValid() || !command.folder.IsValid())
    {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    auto size = static_cast<USHORT>(command.folder.GetSize());
    auto buffer = (PWCHAR)command.folder.GetNakedPointer();
    UNICODE_STRING folder{size, size, buffer};

    auto status = (Blacklist) ? gDrvData.BlackList->Blacklist(&folder)
                              : gDrvData.BlackList->Whitelist(&folder);
    return status;
}

NTSTATUS 
CommandOnSetConfiguration(
    _Inout_ Cpp::ShallowStream& InputStream,
    _Inout_ Cpp::Stream& OutputStream
)
{
    KmUmShared::CommandSetConfiguration command;
    KmUmShared::CommandReplyUpdateFeature reply;

    command.Deserialize(InputStream);
    if (!InputStream.IsValid())
    {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    auto status = gDrvData.ConfigurationManager->SetConfiguration(command.configuration);

    reply.featuresConfiguration = gDrvData.ConfigurationManager->GetCurrentConfiguration();
    reply.Serialize(OutputStream);

    return status;
}
