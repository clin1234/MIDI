// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License
// ============================================================================
// This is part of the Windows MIDI Services App API and should be used
// in your Windows application via an official binary distribution.
// Further information: https://github.com/microsoft/MIDI/
// ============================================================================

#include "pch.h"
#include "MidiSystemExclusiveMessage.h"
#include "MidiSystemExclusiveMessage.g.cpp"


namespace winrt::MIDI_ROOT_NAMESPACE_CPP::implementation
{
    _Use_decl_annotations_
    MidiSystemExclusiveMessage::MidiSystemExclusiveMessage(streams::IBuffer const& /*rawData*/)
    {
        throw hresult_not_implemented();
    }



    // TODO: Need to implement this and any methods required to populate it
    streams::IBuffer MidiSystemExclusiveMessage::RawData()
    {
        return nullptr;
    }
}
