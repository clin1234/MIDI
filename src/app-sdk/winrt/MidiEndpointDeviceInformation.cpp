// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License
// ============================================================================
// This is part of the Windows MIDI Services App SDK and should be used
// in your Windows application via an official binary distribution.
// Further information: https://aka.ms/midi
// ============================================================================


#include "pch.h"
#include "MidiEndpointDeviceInformation.h"
#include "MidiEndpointDeviceInformation.g.cpp"

namespace winrt::Microsoft::Windows::Devices::Midi2::implementation
{
    winrt::hstring MidiEndpointDeviceInformation::ToString()
    {
        winrt::hstring baseName{ };

        if (Name().empty())
        {
            // TODO: Move to resources
            baseName = baseName + L"Unnamed endpoint";
        }
        else
        {
            baseName = baseName + Name();
        }

        return baseName;
    }

#define MIDI_DEVICE_PARENT_PROPERTY_KEY L"System.Devices.Parent"

    winrt::Windows::Devices::Enumeration::DeviceInformation MidiEndpointDeviceInformation::GetContainerDeviceInformation() const noexcept
    {
        // find the container with the same container ID and DeviceInstanceId as us.

        if (ContainerId() == winrt::guid{}) return nullptr;

        auto container = winrt::Windows::Devices::Enumeration::DeviceInformation::CreateFromIdAsync(
            internal::GuidToString(ContainerId()),
            {
                L"System.Devices.DeviceInstanceId",
                MIDI_DEVICE_PARENT_PROPERTY_KEY,
                L"System.Devices.Manufacturer",
                L"System.Devices.ModelName"
            },
            winrt::Windows::Devices::Enumeration::DeviceInformationKind::DeviceContainer).get();

        return container;
    }


    // this has to jump through some hoops because the parent id isn't returned on a normal "CreateFromIdAsync" call
    // even when you specify it. So instead, you have to find the container, then find the child of the container 
    // that matches this device, then call the FindAllAsync to get the actual device object. That object then
    // has the parent id, whcih you can use to find the parent.
    winrt::Windows::Devices::Enumeration::DeviceInformation MidiEndpointDeviceInformation::GetParentDeviceInformation() const noexcept
    {
        try
        {
            const winrt::hstring rootParent = L"HTREE\\ROOT\\0";

            auto container = GetContainerDeviceInformation();

            // TODO: This approach needs to be verified with USB.


            //auto devices = winrt::Windows::Devices::Enumeration::DeviceInformation::FindAllAsync(
            //    L"System.Devices.ContainerId:=\"" + container.Id() +  L"\"" + 
            //    L" AND System.Devices.DeviceInstanceId:=\"" + DeviceInstanceId() + L"\"",
            //    {
            //        MIDI_DEVICE_PARENT_PROPERTY_KEY
            //    },
            //    winrt::Windows::Devices::Enumeration::DeviceInformationKind::Device
            //).get();


            auto myDevice = winrt::Windows::Devices::Enumeration::DeviceInformation::CreateFromIdAsync(
                DeviceInstanceId(),
                {
                    MIDI_DEVICE_PARENT_PROPERTY_KEY,
                    L"System.Devices.DeviceManufacturer",
                    L"System.Devices.ModelName",
                    L"System.Devices.HardwareIds",
                    L"System.Devices.InterfaceClassGuid",
                },
                winrt::Windows::Devices::Enumeration::DeviceInformationKind::Device
            ).get();


            if (myDevice != nullptr)
            {
                //auto parentDevice = devices.GetAt(0);

                if (myDevice.Properties().HasKey(MIDI_DEVICE_PARENT_PROPERTY_KEY))
                {
                    auto parentId = winrt::unbox_value<winrt::hstring>(myDevice.Properties().Lookup(MIDI_DEVICE_PARENT_PROPERTY_KEY));

                    // if the parent is the system root, don't bother trying to resolve that 
                    if (parentId != rootParent)
                    {
                        auto parents = winrt::Windows::Devices::Enumeration::DeviceInformation::FindAllAsync(
                            L"System.Devices.DeviceInstanceId:=\"" + parentId + L"\"",
                            {
                                MIDI_DEVICE_PARENT_PROPERTY_KEY,
                                L"System.Devices.DeviceManufacturer",
                                L"System.Devices.ModelName",
                                L"System.Devices.HardwareIds",
                                L"System.Devices.InterfaceClassGuid",
                                /*L"System.Devices.Manufacturer"
                                L"System.Devices.ModelNumber",
                                L"System.Devices.ModelId",
                                L"System.Devices.Paired",
                                L"System.Devices.Present",
                                L"System.Devices.Connected",
                                L"System.Devices.Category",
                                L"System.Devices.ClassGuid",
                                L"System.Devices.DeviceHasProblem",*/
                            },
                            winrt::Windows::Devices::Enumeration::DeviceInformationKind::Device).get();

                        if (parents != nullptr && parents.Size() == 1)
                        {
                            return parents.GetAt(0);
                        }
                    }
                }
                else
                {
                    // for USB, and some other types, the actual device is the correct parent
                    return myDevice;
                }

            }
        }
        catch (...)
        {
            LOG_IF_FAILED(E_FAIL);   // this also generates a fallback error with file and line number info

            TraceLoggingWrite(
                Midi2SdkTelemetryProvider::Provider(),
                MIDI_SDK_TRACE_EVENT_ERROR,
                TraceLoggingString(__FUNCTION__, MIDI_SDK_TRACE_LOCATION_FIELD),
                TraceLoggingLevel(WINEVENT_LEVEL_ERROR),
                TraceLoggingPointer(this, MIDI_SDK_TRACE_THIS_FIELD),
                TraceLoggingWideString(L"Exception finding the parent of the MIDI Endpoint device.", MIDI_SDK_TRACE_MESSAGE_FIELD),
                TraceLoggingWideString(m_id.c_str(), MIDI_SDK_TRACE_ENDPOINT_DEVICE_ID_FIELD)
            );
        }

        return nullptr;
    }


    collections::IVectorView<winrt::hstring> MidiEndpointDeviceInformation::GetAdditionalPropertiesList() noexcept
    {
        auto additionalProperties = winrt::single_threaded_vector<winrt::hstring>();

        additionalProperties.Append(L"System.ItemNameDisplay");
        additionalProperties.Append(L"System.Devices.FriendlyName");
        additionalProperties.Append(MIDI_DEVICE_PARENT_PROPERTY_KEY);
        additionalProperties.Append(L"System.Devices.DeviceManufacturer");
        additionalProperties.Append(L"System.Devices.InterfaceClassGuid");
        additionalProperties.Append(L"System.Devices.Present");
        additionalProperties.Append(L"System.Devices.InterfaceEnabled");

        // Basics ============================================================================
        additionalProperties.Append(STRING_PKEY_MIDI_TransportLayer);
        additionalProperties.Append(STRING_PKEY_MIDI_TransportCode);

        additionalProperties.Append(STRING_PKEY_MIDI_NativeDataFormat);
        additionalProperties.Append(STRING_PKEY_MIDI_SupportsMulticlient);
        additionalProperties.Append(STRING_PKEY_MIDI_SupportedDataFormats);

        additionalProperties.Append(STRING_PKEY_MIDI_ManufacturerName);
        additionalProperties.Append(STRING_PKEY_MIDI_GenerateIncomingTimestamp);

        additionalProperties.Append(STRING_PKEY_MIDI_EndpointName);
        additionalProperties.Append(STRING_PKEY_MIDI_Description);



        // USB / KS Properties ===============================================================
        additionalProperties.Append(STRING_PKEY_MIDI_GroupTerminalBlocks);
        additionalProperties.Append(STRING_PKEY_MIDI_AssociatedUMP);
        additionalProperties.Append(STRING_PKEY_MIDI_SerialNumber);
        additionalProperties.Append(STRING_PKEY_MIDI_UsbVID);
        additionalProperties.Append(STRING_PKEY_MIDI_UsbPID);


        // Major Known Endpoint Types ========================================================
        additionalProperties.Append(STRING_PKEY_MIDI_EndpointDevicePurpose);

//        additionalProperties.Append(STRING_PKEY_MIDI_EndpointRequiresMetadataHandler);

        // In-protocol Endpoint information ==================================================
        additionalProperties.Append(STRING_PKEY_MIDI_EndpointSupportsMidi2Protocol);
        additionalProperties.Append(STRING_PKEY_MIDI_EndpointSupportsMidi1Protocol);
        additionalProperties.Append(STRING_PKEY_MIDI_EndpointSupportsReceivingJRTimestamps);
        additionalProperties.Append(STRING_PKEY_MIDI_EndpointSupportsSendingJRTimestamps);
        additionalProperties.Append(STRING_PKEY_MIDI_EndpointUmpVersionMajor);
        additionalProperties.Append(STRING_PKEY_MIDI_EndpointUmpVersionMinor);

        additionalProperties.Append(STRING_PKEY_MIDI_EndpointProvidedName);
        additionalProperties.Append(STRING_PKEY_MIDI_EndpointProvidedProductInstanceId);
        additionalProperties.Append(STRING_PKEY_MIDI_DeviceIdentity);

        // In-protocol Endpoint configuration ================================================
        additionalProperties.Append(STRING_PKEY_MIDI_EndpointConfiguredProtocol);
        additionalProperties.Append(STRING_PKEY_MIDI_EndpointConfiguredToSendJRTimestamps);
        additionalProperties.Append(STRING_PKEY_MIDI_EndpointConfiguredToReceiveJRTimestamps);

        // Function Blocks ===================================================================
        additionalProperties.Append(STRING_PKEY_MIDI_FunctionBlockDeclaredCount);
        additionalProperties.Append(STRING_PKEY_MIDI_FunctionBlocksAreStatic);

        // User-supplied metadata ============================================================
        additionalProperties.Append(STRING_PKEY_MIDI_CustomEndpointName);
        additionalProperties.Append(STRING_PKEY_MIDI_CustomLargeImagePath);
        additionalProperties.Append(STRING_PKEY_MIDI_CustomSmallImagePath);
        additionalProperties.Append(STRING_PKEY_MIDI_CustomDescription);
       
        // Additional Capabilities ============================================================
        additionalProperties.Append(STRING_PKEY_MIDI_RequiresNoteOffTranslation);
        additionalProperties.Append(STRING_PKEY_MIDI_RecommendedCCAutomationIntervalMS);
        additionalProperties.Append(STRING_PKEY_MIDI_SupportsMidiPolyphonicExpression);

        // In-protocol Metadata Timestamps ====================================================

        additionalProperties.Append(STRING_PKEY_MIDI_EndpointProvidedProductInstanceIdLastUpdateTime);
        additionalProperties.Append(STRING_PKEY_MIDI_EndpointProvidedNameLastUpdateTime);
        additionalProperties.Append(STRING_PKEY_MIDI_EndpointConfigurationLastUpdateTime);
        additionalProperties.Append(STRING_PKEY_MIDI_EndpointInformationLastUpdateTime);
        additionalProperties.Append(STRING_PKEY_MIDI_DeviceIdentityLastUpdateTime);
        additionalProperties.Append(STRING_PKEY_MIDI_FunctionBlocksLastUpdateTime);


        additionalProperties.Append(STRING_PKEY_MIDI_MidiOutCalculatedLatencyTicks);
        additionalProperties.Append(STRING_PKEY_MIDI_MidiOutCustomLatencyTicks);
        additionalProperties.Append(STRING_PKEY_MIDI_MidiOutLatencyTicksUserOverride);


        additionalProperties.Append(STRING_PKEY_MIDI_VirtualMidiEndpointAssociator);

        // WinMM / Naming properties ==========================================================
  //      additionalProperties.Append(STRING_PKEY_MIDI_UseLegacyMidi1PortNamingScheme);
        additionalProperties.Append(STRING_PKEY_MIDI_CreateMidi1PortsForEndpoint);
        additionalProperties.Append(STRING_PKEY_MIDI_Midi1PortNamingSelection);
        additionalProperties.Append(STRING_PKEY_MIDI_Midi1PortNameTable);

        // these can be useful for debugging, but not much else. They are defined in MidiKSDef.h
        //additionalProperties.Append(STRING_DEVPKEY_KsMidiPort_KsFilterInterfaceId);
        //additionalProperties.Append(STRING_DEVPKEY_KsMidiPort_KsPinId);
        //additionalProperties.Append(STRING_DEVPKEY_KsMidiPort_InPinId);
        //additionalProperties.Append(STRING_DEVPKEY_KsMidiPort_OutPinId);
        //additionalProperties.Append(STRING_DEVPKEY_KsTransport);
        //additionalProperties.Append(STRING_DEVPKEY_KsMidiGroupPinMap);

        



        // Calculated metrics =================================================================
        // we don't load them here because they would spam device information update events
        // and they are (potentially) used only in the service


        // Function Blocks =================================================================


        for (uint8_t fb = 0; fb < MIDI_MAX_FUNCTION_BLOCKS; fb++)
        {
            winrt::hstring functionBlockProperty = internal::BuildFunctionBlockPropertyKey(fb);
            winrt::hstring functionBlockNameProperty = internal::BuildFunctionBlockNamePropertyKey(fb);

            additionalProperties.Append(functionBlockProperty);
            additionalProperties.Append(functionBlockNameProperty);
        }

        return additionalProperties.GetView();
    }


    _Use_decl_annotations_
    bool MidiEndpointDeviceInformation::DeviceMatchesFilter(
        midi2::MidiEndpointDeviceInformation const& deviceInformation,
        midi2::MidiEndpointDeviceInformationFilters const& endpointFilters) noexcept
    {
        // check if diagnostic loopback 
        if (deviceInformation.EndpointPurpose() == MidiEndpointDevicePurpose::DiagnosticLoopback)
        {
            if ((endpointFilters & midi2::MidiEndpointDeviceInformationFilters::DiagnosticLoopback) ==
                midi2::MidiEndpointDeviceInformationFilters::DiagnosticLoopback)
            {
                return true;
            }
        }

        // check if diagnostic ping 
        else if (deviceInformation.EndpointPurpose() == MidiEndpointDevicePurpose::DiagnosticPing)
        {
            if ((endpointFilters & midi2::MidiEndpointDeviceInformationFilters::DiagnosticPing) ==
                midi2::MidiEndpointDeviceInformationFilters::DiagnosticPing)
            {
                return true;
            }
        }

        // check if virtual device responder
        else if (deviceInformation.EndpointPurpose() == MidiEndpointDevicePurpose::VirtualDeviceResponder)
        {
            if ((endpointFilters & midi2::MidiEndpointDeviceInformationFilters::VirtualDeviceResponder) ==
                midi2::MidiEndpointDeviceInformationFilters::VirtualDeviceResponder)
            {
                return true;
            }
        }


        // check if normal client MIDI 1.0 / bytestream
        else if ((deviceInformation.EndpointPurpose() == MidiEndpointDevicePurpose::NormalMessageEndpoint) &&
            (deviceInformation.GetTransportSuppliedInfo().NativeDataFormat == MidiEndpointNativeDataFormat::Midi1ByteFormat))
        {
            if ((endpointFilters & midi2::MidiEndpointDeviceInformationFilters::StandardNativeMidi1ByteFormat) ==
                midi2::MidiEndpointDeviceInformationFilters::StandardNativeMidi1ByteFormat)
            {
                return true;
            }
        }

        // check if normal client MIDI 2.0 / UMP 
        else if ((deviceInformation.EndpointPurpose() == MidiEndpointDevicePurpose::NormalMessageEndpoint) &&
            (deviceInformation.GetTransportSuppliedInfo().NativeDataFormat == MidiEndpointNativeDataFormat::UniversalMidiPacketFormat ||
                deviceInformation.GetTransportSuppliedInfo().NativeDataFormat == MidiEndpointNativeDataFormat::Unknown))
        {
            if ((endpointFilters & midi2::MidiEndpointDeviceInformationFilters::StandardNativeUniversalMidiPacketFormat) == 
                midi2::MidiEndpointDeviceInformationFilters::StandardNativeUniversalMidiPacketFormat)
            {
                return true;
            }
        }

        // no idea what kind of endpoint this is, so default to including it
        else
        {
            return true;
        }

        return false;
    }


    _Use_decl_annotations_
    collections::IVectorView<midi2::MidiEndpointDeviceInformation> MidiEndpointDeviceInformation::FindAll(
        midi2::MidiEndpointDeviceInformationSortOrder const& sortOrder, 
        midi2::MidiEndpointDeviceInformationFilters const& endpointFilters) noexcept
    {
        std::vector<midi2::MidiEndpointDeviceInformation> midiDevices{};

        try
        {
            auto devices = winrt::Windows::Devices::Enumeration::DeviceInformation::FindAllAsync(
                MidiEndpointConnection::GetDeviceSelector(),
                GetAdditionalPropertiesList(),
                winrt::Windows::Devices::Enumeration::DeviceInformationKind::DeviceInterface
            ).get();


            if (devices != nullptr)
            {
                for (auto const& di : devices)
                {
                    auto midiDevice = winrt::make_self<MidiEndpointDeviceInformation>();

                    midiDevice->UpdateFromDeviceInformation(di);

                    if (DeviceMatchesFilter(*midiDevice, endpointFilters))
                    {
                        midiDevices.push_back(*midiDevice);
                    }

                }
            }
        }
        catch (...)
        {
            LOG_IF_FAILED(E_FAIL);   // this also generates a fallback error with file and line number info

            TraceLoggingWrite(
                Midi2SdkTelemetryProvider::Provider(),
                MIDI_SDK_TRACE_EVENT_ERROR,
                TraceLoggingString(__FUNCTION__, MIDI_SDK_TRACE_LOCATION_FIELD),
                TraceLoggingLevel(WINEVENT_LEVEL_ERROR),
                TraceLoggingPointer(MIDI_SDK_STATIC_THIS_PLACEHOLDER_FIELD_VALUE, MIDI_SDK_TRACE_THIS_FIELD),
                TraceLoggingWideString(L"Exception retrieving list of matching MIDI Endpoint devices.", MIDI_SDK_TRACE_MESSAGE_FIELD)
            );
        }


        switch (sortOrder)
        {
        case MidiEndpointDeviceInformationSortOrder::Name:
            std::sort(begin(midiDevices), end(midiDevices),
                [](_In_ const auto& device1, _In_ const auto& device2)
                {
                    return device1.Name() < device2.Name();
                });
            break;

        case MidiEndpointDeviceInformationSortOrder::DeviceInstanceId:
            std::sort(begin(midiDevices), end(midiDevices),
                [](_In_ const auto& device1, _In_ const auto& device2)
                {
                    return device1.DeviceInstanceId() < device2.DeviceInstanceId();
                });
            break;

        case MidiEndpointDeviceInformationSortOrder::EndpointDeviceId:
            std::sort(begin(midiDevices), end(midiDevices),
                [](_In_ const auto& device1, _In_ const auto& device2)
                {
                    return device1.EndpointDeviceId() < device2.EndpointDeviceId();
                });
            break;

        case MidiEndpointDeviceInformationSortOrder::ContainerThenName:
            std::sort(begin(midiDevices), end(midiDevices),
                [](_In_ const auto& device1, _In_ const auto& device2)
                {
                    if (device1.ContainerId() == device2.ContainerId())
                        return device1.Name() < device2.Name();

                    return device1.ContainerId() < device2.ContainerId();
                });
            break;

        case MidiEndpointDeviceInformationSortOrder::ContainerThenDeviceInstanceId:
            std::sort(begin(midiDevices), end(midiDevices),
                [](_In_ const auto& device1, _In_ const auto& device2)
                {
                    if (device1.ContainerId() == device2.ContainerId())
                        return device1.DeviceInstanceId() < device2.DeviceInstanceId();

                    return device1.ContainerId() < device2.ContainerId();
                });
            break;

        case MidiEndpointDeviceInformationSortOrder::ContainerThenEndpointDeviceId:
            std::sort(begin(midiDevices), end(midiDevices),
                [](_In_ const auto& device1, _In_ const auto& device2)
                {
                    if (device1.ContainerId() == device2.ContainerId())
                        return device1.EndpointDeviceId() < device2.EndpointDeviceId();

                    return device1.ContainerId() < device2.ContainerId();
                });
            break;

        case MidiEndpointDeviceInformationSortOrder::TransportMnemonicThenName:
            std::sort(begin(midiDevices), end(midiDevices),
                [](_In_ const auto& device1, _In_ const auto& device2)
                {
                    if (device1.GetTransportSuppliedInfo().TransportCode == device2.GetTransportSuppliedInfo().TransportCode)
                        return device1.Name() < device2.Name();

                    return device1.GetTransportSuppliedInfo().TransportCode < device2.GetTransportSuppliedInfo().TransportCode;
                });
            break;

        case MidiEndpointDeviceInformationSortOrder::TransportMnemonicThenEndpointDeviceId:
            std::sort(begin(midiDevices), end(midiDevices),
                [](_In_ const auto& device1, _In_ const auto& device2)
                {
                    if (device1.GetTransportSuppliedInfo().TransportCode == device2.GetTransportSuppliedInfo().TransportCode)
                        return device1.EndpointDeviceId() < device2.EndpointDeviceId();

                    return device1.GetTransportSuppliedInfo().TransportCode < device2.GetTransportSuppliedInfo().TransportCode;
                });
            break;

        case MidiEndpointDeviceInformationSortOrder::TransportMnemonicThenDeviceInstanceId:
            std::sort(begin(midiDevices), end(midiDevices),
                [](_In_ const auto& device1, _In_ const auto& device2)
                {
                    if (device1.GetTransportSuppliedInfo().TransportCode == device2.GetTransportSuppliedInfo().TransportCode)
                        return device1.DeviceInstanceId() < device2.DeviceInstanceId();

                    return device1.GetTransportSuppliedInfo().TransportCode < device2.GetTransportSuppliedInfo().TransportCode;
                });
            break;


        case MidiEndpointDeviceInformationSortOrder::None:
            // do nothing
            break;

        }

        auto winrtCollection = winrt::single_threaded_observable_vector<midi2::MidiEndpointDeviceInformation>(std::move(midiDevices));

        return winrtCollection.GetView();
    }

    _Use_decl_annotations_
    collections::IVectorView<midi2::MidiEndpointDeviceInformation> MidiEndpointDeviceInformation::FindAll(
        midi2::MidiEndpointDeviceInformationSortOrder const& sortOrder) noexcept
    {
        return FindAll(sortOrder, 
            MidiEndpointDeviceInformationFilters::AllStandardEndpoints
        );
    }

    collections::IVectorView<midi2::MidiEndpointDeviceInformation> MidiEndpointDeviceInformation::FindAll() noexcept
    {
        return FindAll(midi2::MidiEndpointDeviceInformationSortOrder::Name);
    }


    _Use_decl_annotations_
    midi2::MidiEndpointDeviceInformation MidiEndpointDeviceInformation::CreateFromEndpointDeviceId(
        winrt::hstring const& endpointDeviceId) noexcept
    {
        try
        {
            auto cleanedEndpointDeviceId = internal::NormalizeEndpointInterfaceIdHStringCopy(endpointDeviceId);

            // this helps ensure we aren't just using random ids
            if (!internal::IsValidWindowsMidiServicesEndpointId(cleanedEndpointDeviceId))
            {
                return nullptr;
            }

            // we don't like to show stale info. Only return info if device is present
            if (!internal::IsWindowsMidiServicesEndpointPresent(cleanedEndpointDeviceId))
            {
                return nullptr;
            }


            auto di = enumeration::DeviceInformation::CreateFromIdAsync(
                cleanedEndpointDeviceId, 
                GetAdditionalPropertiesList(),
                enumeration::DeviceInformationKind::DeviceInterface
            ).get();

            if (di == nullptr)
            {
                return nullptr;
            }

            // check to see if the interface is enabled
            if (di != nullptr)
            {
                auto endpointInfo = winrt::make_self<MidiEndpointDeviceInformation>();

                endpointInfo->InternalUpdateFromDeviceInformation(di);

                return *endpointInfo;
            }
        }
        catch (...)
        {
            LOG_IF_FAILED(E_FAIL);   // this also generates a fallback error with file and line number info

            TraceLoggingWrite(
                Midi2SdkTelemetryProvider::Provider(),
                MIDI_SDK_TRACE_EVENT_ERROR,
                TraceLoggingString(__FUNCTION__, MIDI_SDK_TRACE_LOCATION_FIELD),
                TraceLoggingLevel(WINEVENT_LEVEL_ERROR),
                TraceLoggingPointer(MIDI_SDK_STATIC_THIS_PLACEHOLDER_FIELD_VALUE, MIDI_SDK_TRACE_THIS_FIELD),
                TraceLoggingWideString(L"Exception creating and updating device.", MIDI_SDK_TRACE_MESSAGE_FIELD),
                TraceLoggingWideString(endpointDeviceId.c_str(), MIDI_SDK_TRACE_ENDPOINT_DEVICE_ID_FIELD)
            );
        }

        return nullptr;
    }




    _Use_decl_annotations_
    foundation::DateTime MidiEndpointDeviceInformation::GetDateTimeProperty(
        winrt::hstring key,
        foundation::DateTime defaultValue) const noexcept
    {
        if (!m_properties.HasKey(key)) return defaultValue;
        if (m_properties.Lookup(key) == nullptr) return defaultValue;

        try
        {
            auto propValue = m_properties.Lookup(key).as<winrt::Windows::Foundation::IPropertyValue>();

            if (propValue != nullptr)
            {
                auto ty = propValue.Type();

                if (ty == foundation::PropertyType::DateTime)
                {
                    auto val = propValue.GetDateTime();

                    return val;
                }
            }

        }
        catch (...)
        {
        }

        return defaultValue;

    }





    winrt::hstring MidiEndpointDeviceInformation::Name() const noexcept
    {
        // user-supplied name overrides all others
        if (GetUserSuppliedInfo().Name != L"") return GetUserSuppliedInfo().Name;

        // endpoint name discovered in-protocol is next highest
        if (GetDeclaredEndpointInfo().Name != L"") return GetDeclaredEndpointInfo().Name;

        // transport-supplied name is last
        if (GetTransportSuppliedInfo().Name != L"") return GetTransportSuppliedInfo().Name;

        return internal::GetDeviceInfoProperty<winrt::hstring>(m_properties, L"System.ItemNameDisplay", L"(Unknown)");
    }

    midi2::MidiEndpointDevicePurpose MidiEndpointDeviceInformation::EndpointPurpose() const noexcept
    {
        // This assumes we're keeping things in sync with the service
        // like we should be

        return (midi2::MidiEndpointDevicePurpose)internal::GetDeviceInfoProperty<uint32_t>(m_properties, STRING_PKEY_MIDI_EndpointDevicePurpose, 0);
    }



    collections::IVectorView<midi2::MidiFunctionBlock> MidiEndpointDeviceInformation::GetDeclaredFunctionBlocks() const noexcept
    {
        auto blocks = winrt::single_threaded_vector<midi2::MidiFunctionBlock>();

        try
        {
            auto functionBlockCount = GetDeclaredEndpointInfo().DeclaredFunctionBlockCount;

            for (uint8_t fb = 0; fb < MIDI_MAX_FUNCTION_BLOCKS && fb < functionBlockCount; fb++)
            {
                winrt::hstring functionBlockProperty = internal::BuildFunctionBlockPropertyKey(fb);
                winrt::hstring functionBlockNameProperty = internal::BuildFunctionBlockNamePropertyKey(fb);

                if (auto refArray = GetBinaryProperty(functionBlockProperty); refArray != nullptr)
                {
                    auto data = refArray.Value();
                    auto arraySize = data.size();

                    auto block = winrt::make_self<midi2::implementation::MidiFunctionBlock>();

                    if (arraySize == sizeof(MidiFunctionBlockProperty))
                    {
                        MidiFunctionBlockProperty prop;

                        memcpy(&prop, data.data(), arraySize);

                        block->UpdateFromDevPropertyStruct(prop);
                        block->InternalSetName(internal::GetDeviceInfoProperty<winrt::hstring>(m_properties, functionBlockNameProperty, L""));
                    }

                    blocks.Append(*block);
                }
            }
        }
        catch (...)
        {
            LOG_IF_FAILED(E_FAIL);   // this also generates a fallback error with file and line number info

            TraceLoggingWrite(
                Midi2SdkTelemetryProvider::Provider(),
                MIDI_SDK_TRACE_EVENT_ERROR,
                TraceLoggingString(__FUNCTION__, MIDI_SDK_TRACE_LOCATION_FIELD),
                TraceLoggingLevel(WINEVENT_LEVEL_ERROR),
                TraceLoggingPointer(this, MIDI_SDK_TRACE_THIS_FIELD),
                TraceLoggingWideString(L"Exception reading function blocks.", MIDI_SDK_TRACE_MESSAGE_FIELD),
                TraceLoggingWideString(m_id.c_str(), MIDI_SDK_TRACE_ENDPOINT_DEVICE_ID_FIELD)
            );

            // we let the blocks get returned outside the handler
        }

        return blocks.GetView();
    }


    collections::IVectorView<midi2::MidiGroupTerminalBlock> MidiEndpointDeviceInformation::GetGroupTerminalBlocks() const noexcept
    {
        return m_groupTerminalBlocks.GetView();
    }


    midi2::MidiEndpointUserSuppliedInfo MidiEndpointDeviceInformation::GetUserSuppliedInfo() const noexcept
    {
        midi2::MidiEndpointUserSuppliedInfo info{};

        info.Name = internal::GetDeviceInfoProperty<winrt::hstring>(m_properties, STRING_PKEY_MIDI_CustomEndpointName, L"");
        info.Description = internal::GetDeviceInfoProperty<winrt::hstring>(m_properties, STRING_PKEY_MIDI_CustomDescription, L"");
        
        info.LargeImagePath = internal::GetDeviceInfoProperty<winrt::hstring>(m_properties, STRING_PKEY_MIDI_CustomLargeImagePath, L"");
        info.SmallImagePath = internal::GetDeviceInfoProperty<winrt::hstring>(m_properties, STRING_PKEY_MIDI_CustomSmallImagePath, L"");

        info.RequiresNoteOffTranslation = internal::GetDeviceInfoProperty<bool>(m_properties, STRING_PKEY_MIDI_RequiresNoteOffTranslation, false);
        info.SupportsMidiPolyphonicExpression = internal::GetDeviceInfoProperty<bool>(m_properties, STRING_PKEY_MIDI_SupportsMidiPolyphonicExpression, false);
        info.RecommendedControlChangeAutomationIntervalMilliseconds = internal::GetDeviceInfoProperty<uint16_t>(m_properties, STRING_PKEY_MIDI_RecommendedCCAutomationIntervalMS, 0);

        return info;
    }

    midi2::MidiEndpointTransportSuppliedInfo MidiEndpointDeviceInformation::GetTransportSuppliedInfo() const noexcept
    {
        midi2::MidiEndpointTransportSuppliedInfo info{};

        info.Name = internal::GetDeviceInfoProperty<winrt::hstring>(m_properties, STRING_PKEY_MIDI_EndpointName, L"");
        info.Description = internal::GetDeviceInfoProperty<winrt::hstring>(m_properties, STRING_PKEY_MIDI_Description, L"");

//        info.LargeImagePath = GetStringProperty(STRING_PKEY_MIDI_TransportSuppliedLargeImagePath, L"");
//        info.SmallImagePath = GetStringProperty(STRING_PKEY_MIDI_TransportSuppliedSmallImagePath, L"");

        info.TransportId = internal::GetDeviceInfoProperty<winrt::guid>(m_properties, STRING_PKEY_MIDI_TransportLayer, winrt::guid{});
        info.TransportCode = internal::GetDeviceInfoProperty<winrt::hstring>(m_properties, STRING_PKEY_MIDI_TransportCode, L"");
        info.SupportsMultiClient = internal::GetDeviceInfoProperty<bool>(m_properties, STRING_PKEY_MIDI_SupportsMulticlient, true);

        info.VendorId = internal::GetDeviceInfoProperty<uint16_t>(m_properties, STRING_PKEY_MIDI_UsbVID, 0);
        info.ProductId = internal::GetDeviceInfoProperty<uint16_t>(m_properties, STRING_PKEY_MIDI_UsbPID, 0);
        info.SerialNumber = internal::GetDeviceInfoProperty<winrt::hstring>(m_properties, STRING_PKEY_MIDI_SerialNumber, L"");
        info.ManufacturerName = internal::GetDeviceInfoProperty<winrt::hstring>(m_properties, STRING_PKEY_MIDI_ManufacturerName, L"");

        auto formatProperty = internal::GetDeviceInfoProperty<uint8_t>(m_properties, STRING_PKEY_MIDI_NativeDataFormat, 0);

        if (formatProperty == MidiDataFormats::MidiDataFormats_ByteStream)
            info.NativeDataFormat = midi2::MidiEndpointNativeDataFormat::Midi1ByteFormat;
        else if (formatProperty == MidiDataFormats::MidiDataFormats_UMP)
            info.NativeDataFormat = midi2::MidiEndpointNativeDataFormat::UniversalMidiPacketFormat;
        else
            info.NativeDataFormat = midi2::MidiEndpointNativeDataFormat::Unknown;
        
        return info;
    }

    midi2::MidiDeclaredStreamConfiguration MidiEndpointDeviceInformation::GetDeclaredStreamConfiguration() const noexcept
    {
        midi2::MidiDeclaredStreamConfiguration config{};

        auto protocolProperty = internal::GetDeviceInfoProperty<uint8_t>(m_properties, STRING_PKEY_MIDI_EndpointConfiguredProtocol, (uint8_t)MidiProtocol::Default);

        if (protocolProperty == MIDI_PROP_CONFIGURED_PROTOCOL_MIDI1)
            config.Protocol = midi2::MidiProtocol::Midi1;
        else if (protocolProperty == MIDI_PROP_CONFIGURED_PROTOCOL_MIDI2)
            config.Protocol = midi2::MidiProtocol::Midi2;
        else
            config.Protocol = midi2::MidiProtocol::Default;

        config.ReceiveJitterReductionTimestamps = internal::GetDeviceInfoProperty<bool>(m_properties, STRING_PKEY_MIDI_EndpointConfiguredToReceiveJRTimestamps, false);
        config.SendJitterReductionTimestamps = internal::GetDeviceInfoProperty<bool>(m_properties, STRING_PKEY_MIDI_EndpointConfiguredToSendJRTimestamps, false);

        return config;
    }



    midi2::MidiDeclaredDeviceIdentity MidiEndpointDeviceInformation::GetDeclaredDeviceIdentity() const noexcept
    {
        auto key = STRING_PKEY_MIDI_DeviceIdentity;

        midi2::MidiDeclaredDeviceIdentity di{};

        auto refArray = GetBinaryProperty(key);

        if (refArray != nullptr)
        {
            auto data = refArray.Value();
            auto arraySize = data.size();

            if (arraySize == sizeof(MidiDeviceIdentityProperty))
            {
                MidiDeviceIdentityProperty identity;

                memcpy(&identity, data.data(), arraySize);

                di.DeviceFamilyLsb = identity.DeviceFamilyLsb;
                di.DeviceFamilyMsb = identity.DeviceFamilyMsb;

                di.DeviceFamilyModelNumberLsb = identity.DeviceFamilyModelNumberLsb;
                di.DeviceFamilyModelNumberMsb = identity.DeviceFamilyModelNumberMsb;

                di.SoftwareRevisionLevelByte1 = identity.SoftwareRevisionLevelByte1;
                di.SoftwareRevisionLevelByte2 = identity.SoftwareRevisionLevelByte2;
                di.SoftwareRevisionLevelByte3 = identity.SoftwareRevisionLevelByte3;
                di.SoftwareRevisionLevelByte4 = identity.SoftwareRevisionLevelByte4;

                di.SystemExclusiveIdByte1 = identity.ManufacturerSysExIdByte1;
                di.SystemExclusiveIdByte2 = identity.ManufacturerSysExIdByte2;
                di.SystemExclusiveIdByte3 = identity.ManufacturerSysExIdByte3;

                return di;
            }
            else
            {
                LOG_IF_FAILED(E_FAIL);   // this also generates a fallback error with file and line number info

                TraceLoggingWrite(
                    Midi2SdkTelemetryProvider::Provider(),
                    MIDI_SDK_TRACE_EVENT_ERROR,
                    TraceLoggingString(__FUNCTION__, MIDI_SDK_TRACE_LOCATION_FIELD),
                    TraceLoggingLevel(WINEVENT_LEVEL_ERROR),
                    TraceLoggingPointer(this, MIDI_SDK_TRACE_THIS_FIELD),
                    TraceLoggingWideString(L"Unable to read device identity. Size of array is incorrect.", MIDI_SDK_TRACE_MESSAGE_FIELD),
                    TraceLoggingWideString(m_id.c_str(), MIDI_SDK_TRACE_ENDPOINT_DEVICE_ID_FIELD)
                );

            }
        }

        return di;
    }


    midi2::MidiDeclaredEndpointInfo MidiEndpointDeviceInformation::GetDeclaredEndpointInfo() const noexcept
    {
        midi2::MidiDeclaredEndpointInfo info;

        info.Name = internal::GetDeviceInfoProperty<winrt::hstring>(m_properties, STRING_PKEY_MIDI_EndpointProvidedName, L"");
        info.ProductInstanceId = internal::GetDeviceInfoProperty<winrt::hstring>(m_properties, STRING_PKEY_MIDI_EndpointProvidedProductInstanceId, L"");

        info.SupportsMidi10Protocol = internal::GetDeviceInfoProperty<bool>(m_properties, STRING_PKEY_MIDI_EndpointSupportsMidi1Protocol, false);
        info.SupportsMidi20Protocol = internal::GetDeviceInfoProperty<bool>(m_properties, STRING_PKEY_MIDI_EndpointSupportsMidi2Protocol, false);
        info.SupportsReceivingJitterReductionTimestamps = internal::GetDeviceInfoProperty<bool>(m_properties, STRING_PKEY_MIDI_EndpointSupportsReceivingJRTimestamps, false);
        info.SupportsSendingJitterReductionTimestamps = internal::GetDeviceInfoProperty<bool>(m_properties, STRING_PKEY_MIDI_EndpointSupportsSendingJRTimestamps, false);

        info.SpecificationVersionMajor = internal::GetDeviceInfoProperty<uint8_t>(m_properties, STRING_PKEY_MIDI_EndpointUmpVersionMajor, (uint8_t)0);
        info.SpecificationVersionMinor = internal::GetDeviceInfoProperty<uint8_t>(m_properties, STRING_PKEY_MIDI_EndpointUmpVersionMinor, (uint8_t)0);

        info.HasStaticFunctionBlocks = internal::GetDeviceInfoProperty<bool>(m_properties, STRING_PKEY_MIDI_FunctionBlocksAreStatic, false);
        info.DeclaredFunctionBlockCount = internal::GetDeviceInfoProperty<uint8_t>(m_properties, STRING_PKEY_MIDI_FunctionBlockDeclaredCount, (uint8_t)0);

        return info;
    }



    _Use_decl_annotations_
    void MidiEndpointDeviceInformation::InternalUpdateFromDeviceInformation(
        winrt::Windows::Devices::Enumeration::DeviceInformation const& deviceInformation) noexcept
    {
        if (deviceInformation == nullptr) return;

        for (auto&& [key, value] : deviceInformation.Properties())
        {
            // insert does a replace if the key exists in the map

            m_properties.Insert(key, value);

            if (key == STRING_PKEY_MIDI_GroupTerminalBlocks)
            {
                ReadGroupTerminalBlocks();
            }
        }

        m_id = internal::NormalizeEndpointInterfaceIdHStringCopy(deviceInformation.Id().c_str());
    }

    _Use_decl_annotations_
    bool MidiEndpointDeviceInformation::UpdateFromDeviceInformation(
        winrt::Windows::Devices::Enumeration::DeviceInformation const& deviceInformation) noexcept
    {
        if (deviceInformation == nullptr) return false;

        // TODO: If an Id is present, check that the ids (after lowercasing) are the same before allowing this.
        // return false if the ids don't match

        InternalUpdateFromDeviceInformation(deviceInformation);

        return true;
    }


    _Use_decl_annotations_
    bool MidiEndpointDeviceInformation::UpdateFromDeviceInformationUpdate(
        winrt::Windows::Devices::Enumeration::DeviceInformationUpdate const& deviceInformationUpdate) noexcept
    {
        if (deviceInformationUpdate == nullptr) return false;

        // TODO: Check that the ids (after lowercasing) are the same before allowing this.
        // return false if the ids don't match


        for (auto&& [key, value] : deviceInformationUpdate.Properties())
        {
            // insert does a replace if the key exists in the map

            m_properties.Insert(key, value);

            if (key == STRING_PKEY_MIDI_GroupTerminalBlocks)
            {
                // this shouldn't happen because these should be static
                ReadGroupTerminalBlocks();
            }
        }

        // TODO: need to update the name

        return false;
    }


    _Use_decl_annotations_
    foundation::IReferenceArray<uint8_t> MidiEndpointDeviceInformation::GetBinaryProperty(winrt::hstring key) const noexcept
    {
        try
        {
            if (!m_properties.HasKey(key)) return nullptr;
            if (m_properties.Lookup(key) == nullptr) return nullptr;

            auto value = m_properties.Lookup(key).as<foundation::IPropertyValue>();

            auto t = value.Type();

            if (t == foundation::PropertyType::UInt8Array)
            {
                auto refArray = winrt::unbox_value<foundation::IReferenceArray<uint8_t>>(m_properties.Lookup(key));

                return refArray;
            }
            else
            {
                LOG_IF_FAILED(E_FAIL);   // this also generates a fallback error with file and line number info

                TraceLoggingWrite(
                    Midi2SdkTelemetryProvider::Provider(),
                    MIDI_SDK_TRACE_EVENT_ERROR,
                    TraceLoggingString(__FUNCTION__, MIDI_SDK_TRACE_LOCATION_FIELD),
                    TraceLoggingLevel(WINEVENT_LEVEL_ERROR),
                    TraceLoggingPointer(this, MIDI_SDK_TRACE_THIS_FIELD),
                    TraceLoggingWideString(L"Value type is something unexpected.", MIDI_SDK_TRACE_MESSAGE_FIELD),
                    TraceLoggingWideString(m_id.c_str(), MIDI_SDK_TRACE_ENDPOINT_DEVICE_ID_FIELD),
                    TraceLoggingWideString(key.c_str(), MIDI_SDK_TRACE_PROPERTY_KEY_FIELD)
                );

                return nullptr;
            }
        }
        catch (...)
        {
            return nullptr;
        }
        
    }



    void MidiEndpointDeviceInformation::ReadGroupTerminalBlocks()
    {
        try
        {
            // in groups property
            // STRING_PKEY_MIDI_IN_GroupTerminalBlocks

            for (auto key : { STRING_PKEY_MIDI_GroupTerminalBlocks /*, STRING_PKEY_MIDI_OUT_GroupTerminalBlocks*/ })
            {
                auto refArray = GetBinaryProperty(key);

                if (refArray != nullptr)
                {
                    auto data = refArray.Value();
                    auto arraySize = data.size();

                    auto gtbs = internal::ReadGroupTerminalBlocksFromPropertyData(refArray.Value().data(), arraySize);

                    for (auto gtb : gtbs)
                    {
                        auto block = winrt::make_self<MidiGroupTerminalBlock>();
                        block->InternalUpdateFromPropertyData(gtb, Name());

                        m_groupTerminalBlocks.Append(*block);
                    }
                }
            }
        }
        catch (...)
        {
            LOG_IF_FAILED(E_FAIL);   // this also generates a fallback error with file and line number info

            TraceLoggingWrite(
                Midi2SdkTelemetryProvider::Provider(),
                MIDI_SDK_TRACE_EVENT_ERROR,
                TraceLoggingString(__FUNCTION__, MIDI_SDK_TRACE_LOCATION_FIELD),
                TraceLoggingLevel(WINEVENT_LEVEL_ERROR),
                TraceLoggingPointer(this, MIDI_SDK_TRACE_THIS_FIELD),
                TraceLoggingWideString(L"Exception processing endpoint GTB property.", MIDI_SDK_TRACE_MESSAGE_FIELD),
                TraceLoggingWideString(m_id.c_str(), MIDI_SDK_TRACE_ENDPOINT_DEVICE_ID_FIELD)
            );
        }
    }

    // instance method
    collections::IVectorView<midi2::Midi1PortNameTableEntry> MidiEndpointDeviceInformation::GetNameTable() const noexcept
    {

        return nullptr;
    }




    _Use_decl_annotations_
    midi2::MidiEndpointDeviceInformation MidiEndpointDeviceInformation::CreateFromMidi1PortDeviceId(
        winrt::hstring const& deviceId) noexcept
    {
        UNREFERENCED_PARAMETER(deviceId);

        // TODO:
        // look up the MIDI 1 port
        // get the associated UMP
        // create a new MidiEndpointDeviceInformation from that associated UMP id

        return nullptr;
    }



    _Use_decl_annotations_
    midi2::MidiEndpointDeviceInformation MidiEndpointDeviceInformation::CreateFromMidi1PortIndex(
        uint32_t const portIndex,
        midi2::Midi1PortFlow const portFlow) noexcept
    {
        UNREFERENCED_PARAMETER(portIndex);
        UNREFERENCED_PARAMETER(portFlow);

        // this will be inefficient until we get named properties

        // TODO:
        // call GetEndpointDeviceIdForMidi1PortIndex
        // create a new MidiEndpointDeviceInformation from that id


        return nullptr;
    }

    _Use_decl_annotations_
    winrt::hstring MidiEndpointDeviceInformation::GetEndpointDeviceIdForMidi1PortIndex(
        uint32_t const portIndex,
        midi2::Midi1PortFlow const portFlow) noexcept
    {
        UNREFERENCED_PARAMETER(portIndex);
        UNREFERENCED_PARAMETER(portFlow);

        // TODO:
        // Get all active MIDI 1.0 ports for this flow
        // Look through each until you find the portIndex that matches
        // get the associated UMP
        // return the cleaned-up associated UMP value

        return L"";
    }

    _Use_decl_annotations_
    collections::IVectorView<midi2::MidiEndpointDeviceInformation> MidiEndpointDeviceInformation::FindAllForMidi1PortName(
        winrt::hstring const& portName,
        midi2::Midi1PortFlow const portFlow) noexcept
    {
        UNREFERENCED_PARAMETER(portName);
        UNREFERENCED_PARAMETER(portFlow);

        // TODO:
        // call FindAllEndpointDeviceIdsForMidi1PortName
        // instantiate those MidiEndpointDeviceInformation objects

        return nullptr;
    }

    _Use_decl_annotations_
    collections::IVectorView<winrt::hstring> MidiEndpointDeviceInformation::FindAllEndpointDeviceIdsForMidi1PortName(
        winrt::hstring const& portName,
        midi2::Midi1PortFlow const portFlow) noexcept
    {
        UNREFERENCED_PARAMETER(portName);
        UNREFERENCED_PARAMETER(portFlow);

        // TODO:
        // Get all active MIDI 1.0 ports where the friendly name matches
        // return the associated UMP ids from those

        return nullptr;
    }


    // this is for this endpoint only, not a static method
    _Use_decl_annotations_
    midi2::MidiEndpointAssociatedPortDeviceInformation MidiEndpointDeviceInformation::GetAssociatedMidi1PortForGroup(
        midi2::MidiGroup const& group,
        midi2::Midi1PortFlow const portFlow) const noexcept
    {
        UNREFERENCED_PARAMETER(group);
        UNREFERENCED_PARAMETER(portFlow);

        // TODO:
        // get the associated MIDI 1 ports for this endpoint
        // find the one with a matching group and flow
        // return that one only

        return nullptr;
    }


    _Use_decl_annotations_
    collections::IVectorView<midi2::MidiEndpointAssociatedPortDeviceInformation> MidiEndpointDeviceInformation::GetAssociatedMidi1Ports(
        midi2::Midi1PortFlow const portFlow) const noexcept
    {
        winrt::hstring deviceSelector;

        collections::IVector<midi2::MidiEndpointAssociatedPortDeviceInformation>* ports;

        if (portFlow == midi2::Midi1PortFlow::MidiMessageDestination)
        {
            ports = (collections::IVector<midi2::MidiEndpointAssociatedPortDeviceInformation>*)&m_midi1DestinationPorts;
            deviceSelector = winrt::Windows::Devices::Midi::MidiOutPort::GetDeviceSelector();   // could also use DEVINTERFACE_MIDI_OUTPUT
        }
        else
        {
            ports = (collections::IVector<midi2::MidiEndpointAssociatedPortDeviceInformation>*)&m_midi1SourcePorts;
            deviceSelector = winrt::Windows::Devices::Midi::MidiInPort::GetDeviceSelector();    // could also use DEVINTERFACE_MIDI_INPUT
        }

        ports->Clear();

        // we need some additional properties

        auto additionalProperties = winrt::single_threaded_vector<winrt::hstring>();

        additionalProperties.Append(STRING_PKEY_MIDI_AssociatedUMP);
        additionalProperties.Append(STRING_PKEY_MIDI_PortAssignedGroupIndex);
        additionalProperties.Append(STRING_PKEY_MIDI_CustomPortNumber);
        additionalProperties.Append(STRING_PKEY_MIDI_ServiceAssignedPortNumber);

        // TODO: Likely need to modify the filter so it only shows online/connected devices. TBD.
        auto searchResults = enumeration::DeviceInformation::FindAllAsync(deviceSelector, additionalProperties).get();

        if (searchResults != nullptr)
        {
            for (auto const& device : searchResults)
            {
                auto id = internal::SafeGetSwdPropertyFromDeviceInformation<winrt::hstring>(STRING_PKEY_MIDI_AssociatedUMP, device, L"");

                if (internal::NormalizeEndpointInterfaceIdHStringCopy(id) == m_id)
                {
                    // this is one of our child ports, so we add to the results
                    // unfortunately, with GUID-based properties, you cannot include them
                    // in the search query, so we have to get everything and cycle through

                    auto group = midi2::MidiGroup((uint8_t)internal::SafeGetSwdPropertyFromDeviceInformation<UINT32>(STRING_PKEY_MIDI_PortAssignedGroupIndex, device, 0));
                    UINT32 portIndex{ 0 };

                    auto prop = device.Properties().Lookup(STRING_PKEY_MIDI_ServiceAssignedPortNumber);
                    if (prop)
                    {
                        portIndex = winrt::unbox_value<UINT32>(prop);
                    }
                    else
                    {
                        prop = device.Properties().Lookup(STRING_PKEY_MIDI_CustomPortNumber);
                        if (prop)
                        {
                            portIndex = winrt::unbox_value<UINT32>(prop);
                        }
                    }

                    auto port = winrt::make_self<midi2::implementation::MidiEndpointAssociatedPortDeviceInformation>();

                    port->InternalInitialize(
                        this->ContainerId(),
                        this->DeviceInstanceId(),
                        this->EndpointDeviceId(),
                        group,
                        portFlow,
                        device.Name(),
                        device.Id(),
                        portIndex,
                        device
                    );

                    ports->Append(*port);
                }
            }
        }

        // TODO: sort the vector on group index before returning

        return ports->GetView();
    }






}
