// WARNING: Please don't edit this file. It was generated by C++/WinRT v2.0.240405.15

#pragma once
#ifndef WINRT_Microsoft_Windows_Devices_Midi2_ClientPlugins_1_H
#define WINRT_Microsoft_Windows_Devices_Midi2_ClientPlugins_1_H
#include "winrt/impl/Microsoft.Windows.Devices.Midi2.ClientPlugins.0.h"
WINRT_EXPORT namespace winrt::Microsoft::Windows::Devices::Midi2::ClientPlugins
{
    struct WINRT_IMPL_EMPTY_BASES IMidiChannelEndpointListener :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IMidiChannelEndpointListener>
    {
        IMidiChannelEndpointListener(std::nullptr_t = nullptr) noexcept {}
        IMidiChannelEndpointListener(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES IMidiGroupEndpointListener :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IMidiGroupEndpointListener>
    {
        IMidiGroupEndpointListener(std::nullptr_t = nullptr) noexcept {}
        IMidiGroupEndpointListener(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES IMidiMessageTypeEndpointListener :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IMidiMessageTypeEndpointListener>
    {
        IMidiMessageTypeEndpointListener(std::nullptr_t = nullptr) noexcept {}
        IMidiMessageTypeEndpointListener(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
}
#endif