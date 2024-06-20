// WARNING: Please don't edit this file. It was generated by C++/WinRT v2.0.240405.15

#pragma once
#ifndef WINRT_Microsoft_Windows_Devices_Midi2_ServiceConfig_2_H
#define WINRT_Microsoft_Windows_Devices_Midi2_ServiceConfig_2_H
#include "winrt/impl/Microsoft.Windows.Devices.Midi2.ServiceConfig.1.h"
WINRT_EXPORT namespace winrt::Microsoft::Windows::Devices::Midi2::ServiceConfig
{
    struct MidiServiceConfigResponse
    {
        winrt::Microsoft::Windows::Devices::Midi2::ServiceConfig::MidiServiceConfigResponseStatus Status;
        hstring ResponseJson;
    };
    inline bool operator==(MidiServiceConfigResponse const& left, MidiServiceConfigResponse const& right) noexcept
    {
        return left.Status == right.Status && left.ResponseJson == right.ResponseJson;
    }
    inline bool operator!=(MidiServiceConfigResponse const& left, MidiServiceConfigResponse const& right) noexcept
    {
        return !(left == right);
    }
    struct WINRT_IMPL_EMPTY_BASES MidiServiceConfig : winrt::Microsoft::Windows::Devices::Midi2::ServiceConfig::IMidiServiceConfig
    {
        MidiServiceConfig(std::nullptr_t) noexcept {}
        MidiServiceConfig(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Microsoft::Windows::Devices::Midi2::ServiceConfig::IMidiServiceConfig(ptr, take_ownership_from_abi) {}
        static auto UpdateTransportPluginConfig(winrt::Microsoft::Windows::Devices::Midi2::ServiceConfig::IMidiServiceTransportPluginConfig const& configUpdate);
        static auto UpdateProcessingPluginConfig(winrt::Microsoft::Windows::Devices::Midi2::ServiceConfig::IMidiServiceMessageProcessingPluginConfig const& configUpdate);
    };
}
#endif