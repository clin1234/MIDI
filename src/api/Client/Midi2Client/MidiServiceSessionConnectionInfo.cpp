// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License
// ============================================================================
// This is part of the Windows MIDI Services App API and should be used
// in your Windows application via an official binary distribution.
// Further information: https://github.com/microsoft/MIDI/
// ============================================================================

#include "pch.h"
#include "MidiServiceSessionConnectionInfo.h"
#include "MidiServiceSessionConnectionInfo.g.cpp"


namespace winrt::Windows::Devices::Midi2::implementation
{

    _Use_decl_annotations_
    void MidiServiceSessionConnectionInfo::InternalInitialize(
        winrt::hstring const endpointDeviceId,
        uint16_t const instanceCount,
        foundation::DateTime const earliestConnectionTime
        )
    {
        m_endpointDeviceId = internal::NormalizeEndpointInterfaceIdHStringCopy(endpointDeviceId);
        m_instanceCount = instanceCount;
        m_earliestConnectionTime = earliestConnectionTime;
    }



}