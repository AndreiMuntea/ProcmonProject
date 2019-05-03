#include "trace.h"
#include "CommandInterpreter.tmh"
#include "CommandInterpreter.hpp"
#include "GlobalData.hpp"
#include "ProcessUtils.hpp"
#include "Ioctl.hpp"
#include "RemoteThread.hpp"
#include "../Common/FltPortCommand.hpp"

#include <iostream>
#include <functional>
#include <memory>

CommandInterpreter::CommandInterpreter()
{
    availableCommands.emplace(
        std::piecewise_construct,
        std::make_tuple("Help"),
        std::make_tuple("Displays a list of available commands", [this]() {this->PrintHelpCommand(); })
    );

    availableCommands.emplace(
        std::piecewise_construct,
        std::make_tuple("Start"),
        std::make_tuple("Starts a thread pool with 5 threads", [this]() {this->StartThreadPoolCommand(); })
    );

    availableCommands.emplace(
        std::piecewise_construct,
        std::make_tuple("Stop"),
        std::make_tuple("Stops the previously created thread pool", [this]() {this->StopThreadPoolCommand(); })
    );

    availableCommands.emplace(
        std::piecewise_construct,
        std::make_tuple("DumpProcessesToolHelp32"),
        std::make_tuple("Dumps active processes using ToolHelp32Snapshot", PuDumpActiveProcessesToolHelp32Snapshot)
    );

    availableCommands.emplace(
        std::piecewise_construct,
        std::make_tuple("DumpProcessesPsApi"),
        std::make_tuple("Dumps active processes using PsApi", PuDumpActiveProcessesPsapi)
    );

    availableCommands.emplace(
        std::piecewise_construct,
        std::make_tuple("DumpProcessesZwQuerySystemInformation"),
        std::make_tuple("Dumps active processes using ZwQuerySystemInformation", PuDumpActiveProcessesZwQuerySystemInformation)
    );

    availableCommands.emplace(
        std::piecewise_construct,
        std::make_tuple("SendFirstIoctl"),
        std::make_tuple("Sends first ioctl code to the driver", []() {IoctlSendIoctl((DWORD)(FIRST_IOCTL_CODE), nullptr, 0); })
    );

    availableCommands.emplace(
        std::piecewise_construct,
        std::make_tuple("SendSecondIoctl"),
        std::make_tuple("Sends second ioctl code to the driver", []() {IoctlSendIoctl((DWORD)(SECOND_IOCTL_CODE), nullptr, 0); })
    );

    availableCommands.emplace(
        std::piecewise_construct,
        std::make_tuple("ProtectProcess"),
        std::make_tuple("Protects a process with custom pid", [this]() {this->ProtectProcessCommand(); })
    );    
    
    availableCommands.emplace(
        std::piecewise_construct,
        std::make_tuple("CreateRemoteThread"),
        std::make_tuple("Opens a 'cmd' process then creates a remote thread using CreateRemoteThread", RmtCreateRemoteThread)
    );

    availableCommands.emplace(
        std::piecewise_construct,
        std::make_tuple("ConnectFltPort"),
        std::make_tuple("Connects to '\\MyCommunicationPort'", [this]() {this->ConnectFltPortCommand(); })
    );

    availableCommands.emplace(
        std::piecewise_construct,
        std::make_tuple("DisconnectFltPort"),
        std::make_tuple("DisconnectFltPort from '\\MyCommunicationPort'", [this]() {this->DisconnectFltPortCommand(); })
    );

    availableCommands.emplace(
        std::piecewise_construct,
        std::make_tuple("EnableFeature"),
        std::make_tuple("Sends EnableFeature to '\\MyCommunicationPort'", [this]() { this->UpdateFeatureCommand(true); })
    );

    availableCommands.emplace(
        std::piecewise_construct,
        std::make_tuple("DisableFeature"),
        std::make_tuple("Sends DisableFeature to '\\MyCommunicationPort'", [this]() { this->UpdateFeatureCommand(false); })
    );

    availableCommands.emplace(
        std::piecewise_construct,
        std::make_tuple("ProtectFolder"),
        std::make_tuple("Sends ProtectFolder to '\\MyCommunicationPort'", [this]() { this->UpdateBlacklistedFolder(true); })
    );

    availableCommands.emplace(
        std::piecewise_construct,
        std::make_tuple("UnprotectFolder"),
        std::make_tuple("Sends UnprotectFolder to '\\MyCommunicationPort'", [this]() { this->UpdateBlacklistedFolder(false); })
    );


    availableCommands.emplace(
        std::piecewise_construct,
        std::make_tuple("DeleteAtReboot"),
        std::make_tuple("Creates a dummyfile and schedule it to be deleted at reboot", PuDeleteFileAtReboot)
    );

    availableCommands.emplace(
        std::piecewise_construct,
        std::make_tuple("Exit"),
        std::make_tuple("Performs a clean exit of the application", [this]() {this->ExitCommand(); })
    );
}

void 
CommandInterpreter::Execute(
    _In_ const std::string& Command
)
{
    std::cout << "Received command \"" << Command << "\"" << std::endl;
    ConsoleAppLogInfo("Received command : %s", Command.c_str());

    auto cmdHandler = availableCommands.find(Command);
    if (cmdHandler != availableCommands.end())
    {
        ConsoleAppLogInfo("Handler found for command command : %s", Command.c_str());
        try
        {
            cmdHandler->second.Execute();
        }
        catch (std::exception& ex)
        {
            std::cout << "An exception has occured during command execution " << std::string(ex.what()) << std::endl;
            ConsoleAppLogCritical("An exception has occured during command execution : %s", ex.what());
        }
    }
    else
    {
        std::cout << "Handler not found for command \"" << Command << "\"" << std::endl;
        ConsoleAppLogWarning("Handler not found for command : %s", Command.c_str());
    }
}

void 
CommandInterpreter::PrintHelpCommand()
{
    std::cout << "Available commands: " << std::endl;
    for (const auto& command : this->availableCommands)
    {
        std::cout << "\t > " << command.first << " - " << command.second.GetDescription() << std::endl;
    }
}

void 
CommandInterpreter::ExitCommand()
{
    gGlobalData.IsApplicationRunning = false;
}

void 
CommandInterpreter::StartThreadPoolCommand()
{
    if (gGlobalData.ThreadPool)
    {
        std::cout << "ThreadPool is running. Please stop it first" << std::endl;
        ConsoleAppLogWarning("ThreadPool is already running");
        return;
    }

    std::cout << "Creating a thread pool with 5 threads" << std::endl;
    ConsoleAppLogInfo("Creating a thread pool with 5 threads");
    
    gGlobalData.ThreadPool.reset(new ThreadPool(5));
}

void 
CommandInterpreter::StopThreadPoolCommand()
{
    if (!gGlobalData.ThreadPool)
    {
        std::cout << "ThreadPool is not running. Please start it first" << std::endl;
        ConsoleAppLogWarning("ThreadPool is not running");
        return;
    }

    gGlobalData.ThreadPool->Shutdown();
    gGlobalData.ThreadPool = nullptr;
}

void CommandInterpreter::ConnectFltPortCommand()
{
    if (gGlobalData.FltPort && gGlobalData.FltPort->IsConnected())
    {
        std::cout << "FltPort is already connected. Please disconnect it first" << std::endl;
        ConsoleAppLogWarning("FltPort is already connected");
        return;
    }

    gGlobalData.FltPort.reset(new FilterPort(L"\\MyCommunicationPort"));

    unsigned __int64 configuration = 0;
    gGlobalData.ConfigurationRegistryKey->GetValue(configuration);

    std::wcout << "Setting configuration to : " << std::hex << configuration << std::dec;
    SetConfigurationCommand(configuration);
}

void CommandInterpreter::DisconnectFltPortCommand()
{
    if (!gGlobalData.FltPort || !gGlobalData.FltPort->IsConnected())
    {
        std::cout << "FltPort is not connected. Please connect it first" << std::endl;
        ConsoleAppLogWarning("FltPort is not connected");
        return;
    }

    gGlobalData.FltPort->Disconnect();
    gGlobalData.FltPort = nullptr;
}

void 
CommandInterpreter::ProtectProcessCommand()
{
    unsigned __int32 pid = 0;

    std::cout << "Enter process pid: ";
    std::cin >> pid;

    IoctlSendIoctl((DWORD)THIRD_IOCTL_CODE, &pid, sizeof(unsigned __int32));
}

void CommandInterpreter::UpdateFeatureCommand(bool Enable)
{
    if (!gGlobalData.FltPort || !gGlobalData.FltPort->IsConnected())
    {
        std::cout << "Disconnected FLT port!" << std::endl;
        return;
    }

    PrintAvailableFeatures();

    unsigned __int32 feature = 0;
    std::cout << "What feature do you want to update : ";
    std::cin >> feature;

    if (feature > static_cast<unsigned __int32>(KmUmShared::Feature::featureMaxIndex))
    {
        std::cout << "Invalid feature index!" << std::endl;
        return;
    }

    auto command = std::make_shared<KmUmShared::CommandUpdateFeature>();
    auto reply = std::make_shared<KmUmShared::CommandReplyUpdateFeature>();

    command->commandCode = (Enable) ? KmUmShared::CommandCode::commandEnableFeature 
                                    : KmUmShared::CommandCode::commandDisableFeature;
    command->feature = static_cast<KmUmShared::Feature>(feature);

    auto status = gGlobalData.FltPort->Send(command, reply);
    if (status == ERROR_SUCCESS)
    {
        std::cout << "Current features configuration: " << reply->featuresConfiguration << std::endl;
        gGlobalData.ConfigurationRegistryKey->SetValue(reply->featuresConfiguration);
    }
    else
    {
        std::cout << "Couldn't perform operation" << std::endl;
    }
}

void CommandInterpreter::UpdateBlacklistedFolder(bool Blacklist)
{
    if (!gGlobalData.FltPort || !gGlobalData.FltPort->IsConnected())
    {
        std::cout << "Disconnected FLT port!" << std::endl;
        return;
    }

    std::wstring path;
    std::wcout << "What path do you want to update : " << std::endl;
    std::getline(std::wcin, path);

    auto dosPath = PuNtPathToDosPath(path);

    auto command = std::make_shared<KmUmShared::CommandUpdateBlacklistFolder>();
    auto reply = std::make_shared<KmUmShared::CommandReply>();

    command->commandCode = (Blacklist)  ? KmUmShared::CommandCode::commandProtectFolder
                                        : KmUmShared::CommandCode::commandUnprotectFolder;
    command->folder = Cpp::String{ (const unsigned __int8*)dosPath.c_str(), static_cast<unsigned __int32>(dosPath.size()) * sizeof(WCHAR) };

    auto status = gGlobalData.FltPort->Send(command, reply);
    if (status == ERROR_SUCCESS)
    {
        std::wcout << "Successfully updated" << std::endl;
    }
    else
    {
        std::wcout << "Couldn't perform operation" << std::endl;
    }
}

void CommandInterpreter::SetConfigurationCommand(unsigned __int64 Configuration)
{
    if (!gGlobalData.FltPort || !gGlobalData.FltPort->IsConnected())
    {
        std::cout << "Disconnected FLT port!" << std::endl;
        return;
    }
    auto command = std::make_shared<KmUmShared::CommandSetConfiguration>(Configuration);
    auto reply = std::make_shared<KmUmShared::CommandReplyUpdateFeature>();

    auto status = gGlobalData.FltPort->Send(command, reply);
    if (status == ERROR_SUCCESS)
    {
        std::wcout << "Successfully set configuration" << std::endl; 
        std::wcout << "Current features configuration: " << reply->featuresConfiguration << std::endl;
    }
    else
    {
        std::wcout << "Couldn't perform operation. Status = " << std::hex << status << std::dec << std::endl;
    }
}

void CommandInterpreter::PrintAvailableFeatures()
{
    std::cout << "\t> Available features:" << std::endl;
    std::cout << "\t\t> " << static_cast<int>(KmUmShared::Feature::featureMonitorStarted) << " - featureMonitorStarted" << std::endl;
    std::cout << "\t\t> " << static_cast<int>(KmUmShared::Feature::featureMonitorProcessCreate) << " - featureMonitorProcessCreate" << std::endl;
    std::cout << "\t\t> " << static_cast<int>(KmUmShared::Feature::featureMonitorProcessTerminate) << " - featureMonitorProcessTerminate" << std::endl;
    std::cout << "\t\t> " << static_cast<int>(KmUmShared::Feature::featureMonitorThreadCreate) << " - featureMonitorThreadCreate" << std::endl;
    std::cout << "\t\t> " << static_cast<int>(KmUmShared::Feature::featureMonitorThreadTerminate) << " - featureMonitorThreadTerminate" << std::endl;
    std::cout << "\t\t> " << static_cast<int>(KmUmShared::Feature::featureMonitorImageLoaded) << " - featureMonitorImageLoaded" << std::endl;
    std::cout << "\t\t> " << static_cast<int>(KmUmShared::Feature::featureMonitorRegistryCreateKey) << " - featureMonitorRegistryCreateKey" << std::endl;
    std::cout << "\t\t> " << static_cast<int>(KmUmShared::Feature::featureMonitorRegistrySetValue) << " - featureMonitorRegistrySetValue" << std::endl;
    std::cout << "\t\t> " << static_cast<int>(KmUmShared::Feature::featureMonitorRegistryDeleteKey) << " - featureMonitorRegistryDeleteKey" << std::endl;
    std::cout << "\t\t> " << static_cast<int>(KmUmShared::Feature::featureMonitorRegistryDeleteKeyValue) << " - featureMonitorRegistryDeleteKeyValue" << std::endl;
    std::cout << "\t\t> " << static_cast<int>(KmUmShared::Feature::featureMonitorRegistryLoadKey) << " - featureMonitorRegistryLoadKey" << std::endl;
    std::cout << "\t\t> " << static_cast<int>(KmUmShared::Feature::featureMonitorRegistryRenameKey) << " - featureMonitorRegistryRenameKey" << std::endl;
    std::cout << "\t\t> " << static_cast<int>(KmUmShared::Feature::featureMonitorFileCreate) << " - featureMonitorFileCreate" << std::endl;
    std::cout << "\t\t> " << static_cast<int>(KmUmShared::Feature::featureMonitorFileClose) << " - featureMonitorFileClose" << std::endl;
    std::cout << "\t\t> " << static_cast<int>(KmUmShared::Feature::featureMonitorFileCleanup) << " - featureMonitorFileCleanup" << std::endl;
    std::cout << "\t\t> " << static_cast<int>(KmUmShared::Feature::featureMonitorFileRead) << " - featureMonitorFileRead" << std::endl;
    std::cout << "\t\t> " << static_cast<int>(KmUmShared::Feature::featureMonitorFileWrite) << " - featureMonitorFileWrite" << std::endl;
    std::cout << "\t\t> " << static_cast<int>(KmUmShared::Feature::featureMonitorFileSetInformation) << " - featureMonitorFileSetInformation" << std::endl;
}
