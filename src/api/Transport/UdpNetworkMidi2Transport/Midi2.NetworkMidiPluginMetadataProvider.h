// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License
// ============================================================================
// This is part of the Windows MIDI Services App API and should be used
// in your Windows application via an official binary distribution.
// Further information: https://github.com/microsoft/MIDI/
// ============================================================================

#pragma once


class CMidi2NetworkMidiPluginMetadataProvider :
    public Microsoft::WRL::RuntimeClass<
    Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
    IMidiServiceTransportPluginMetadataProvider>

{
public:
    STDMETHOD(Initialize());
    STDMETHOD(GetMetadata(_Out_ PTRANSPORTMETADATA metadata));
    STDMETHOD(Shutdown)();

private:

};

