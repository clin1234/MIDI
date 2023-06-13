// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License
// Further information: https://github.com/microsoft/MIDI/
// ============================================================================

#pragma once
#include "Ump.g.h"


namespace winrt::Microsoft::Devices::Midi2::implementation
{
    struct Ump : UmpT<Ump>
    {
        Ump() = default;

        com_array<uint32_t> Words();
        uint8_t WordCount();

        winrt::Microsoft::Devices::Midi2::MidiMessageType MessageType();
        void MessageType(winrt::Microsoft::Devices::Midi2::MidiMessageType const& value);
    };
}
namespace winrt::Microsoft::Devices::Midi2::factory_implementation
{
    struct Ump : UmpT<Ump, implementation::Ump>
    {
    };
}