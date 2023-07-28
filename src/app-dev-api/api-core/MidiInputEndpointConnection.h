// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License
// ============================================================================
// This is part of the Windows MIDI Services App API and should be used
// in your Windows application via an official binary distribution.
// Further information: https://github.com/microsoft/MIDI/
// ============================================================================

#pragma once

#include "MidiInputEndpointConnection.g.h"
#include "MidiEndpointConnection.h"

#include "MidiMessageReceivedEventArgs.h"

#include "midi_service_interface.h"


namespace winrt::Windows::Devices::Midi2::implementation
{
    struct MidiInputEndpointConnection : MidiInputEndpointConnectionT<MidiInputEndpointConnection, 
        Windows::Devices::Midi2::implementation::MidiEndpointConnection,
        IMidiCallback>
    {
        MidiInputEndpointConnection() = default;

        static hstring GetDeviceSelectorForInput() { return L""; /* TODO */ }

        winrt::Windows::Foundation::Collections::IVector<winrt::Windows::Devices::Midi2::IMidiEndpointMessageListener> MessageListeners() { return m_messageListeners; }



        STDMETHOD(Callback)(_In_ PVOID Data, _In_ UINT Size, _In_ LONGLONG Position) override;

        inline winrt::event_token MessageReceived(winrt::Windows::Foundation::TypedEventHandler<IInspectable, winrt::Windows::Devices::Midi2::MidiMessageReceivedEventArgs> const& handler)
        {
            return m_messagesReceivedEvent.add(handler);
        }

        inline void MessageReceived(winrt::event_token const& token) noexcept
        {
            m_messagesReceivedEvent.remove(token);
        }


        bool InternalStart();

    private:
        winrt::event<winrt::Windows::Foundation::TypedEventHandler<IInspectable, winrt::Windows::Devices::Midi2::MidiMessageReceivedEventArgs>> m_messagesReceivedEvent;

        winrt::Windows::Foundation::Collections::IVector<winrt::Windows::Devices::Midi2::IMidiEndpointMessageListener>
            m_messageListeners{ winrt::single_threaded_vector<winrt::Windows::Devices::Midi2::IMidiEndpointMessageListener>() };

    };
}
namespace winrt::Windows::Devices::Midi2::factory_implementation
{
    struct MidiInputEndpointConnection : MidiInputEndpointConnectionT<MidiInputEndpointConnection, implementation::MidiInputEndpointConnection>
    {
    };
}