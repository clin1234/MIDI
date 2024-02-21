// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License
// ============================================================================
// This is part of the Windows MIDI Services App API and should be used
// in your Windows application via an official binary distribution.
// Further information: https://github.com/microsoft/MIDI/
// ============================================================================

#include "pch.h"
#include "MidiEndpointConnection.h"




namespace winrt::Windows::Devices::Midi2::implementation
{

    _Use_decl_annotations_
    midi2::MidiSendMessageResult MidiEndpointConnection::SendMultipleMessagesBuffer(
        internal::MidiTimestamp timestamp,
        foundation::IMemoryBuffer const& buffer,
        uint32_t byteOffset,
        uint32_t byteCount) noexcept
    {
        internal::LogInfo(__FUNCTION__, L"Enter");

        winrt::Windows::Foundation::IMemoryBufferReference bufferReference = buffer.CreateReference();

        auto interop = bufferReference.as<IMemoryBufferByteAccess>();

        uint8_t* dataPointer{};
        uint32_t dataSize{ };

        if (SUCCEEDED(interop->GetBuffer(&dataPointer, &dataSize)))
        {
            if (byteOffset + byteCount > dataSize)
            {
                internal::LogGeneralError(__FUNCTION__, L"Buffer smaller than provided offset + byteLength (first check)");

                return midi2::MidiSendMessageResult::Failed | midi2::MidiSendMessageResult::DataIndexOutOfRange;
            }


            // make sure we're not going to spin past the end of the buffer
            if (byteOffset + byteCount > bufferReference.Capacity())
            {
                internal::LogGeneralError(__FUNCTION__, L"Buffer smaller than provided offset + byteLength (second check) ");

                return midi2::MidiSendMessageResult::Failed | midi2::MidiSendMessageResult::DataIndexOutOfRange;
            }


            // endianness becomes a concern so we need to make sure we treat this as words
            //uint32_t* wordDataPointer = (uint32_t*)(dataPointer + byteOffset);
            uint8_t* byteDataPointer = dataPointer + byteOffset;
            uint32_t bytesProcessed{ 0 };

            while (true)
            {
                uint8_t bytesInThisMessage = internal::GetUmpLengthInBytesFromFirstWord((uint32_t&)byteDataPointer);

                if (bytesProcessed + bytesInThisMessage <= byteCount)
                {
                    auto result = SendMessageRaw(m_endpointAbstraction, (void*)(byteDataPointer), bytesInThisMessage, timestamp);

                    if (SendMessageFailed(result))
                    {
                        internal::LogGeneralError(__FUNCTION__, L"SendMessageRaw failed");
                        return result;
                    }
                }
                else
                {
                    internal::LogGeneralError(__FUNCTION__, L"Ran out of data");
                    auto returnValue = midi2::MidiSendMessageResult::Failed | midi2::MidiSendMessageResult::DataIndexOutOfRange ;

                    if (bytesProcessed > 0)
                    {
                        returnValue |= midi2::MidiSendMessageResult::MessageListPartiallyProcessed;
                    }

                    return returnValue;
                }

                bytesProcessed += bytesInThisMessage;
                byteDataPointer += bytesInThisMessage;

                if (bytesProcessed >= byteCount)
                {
                    break;
                }
            }

            if (bytesProcessed != byteCount)
            {
                internal::LogGeneralError(__FUNCTION__, L"Ran out of data");
                return midi2::MidiSendMessageResult::Failed | midi2::MidiSendMessageResult::MessageListPartiallyProcessed;

            }
        }
        else
        {
            internal::LogGeneralError(__FUNCTION__, L"Unable to get buffer");

            return midi2::MidiSendMessageResult::Failed | midi2::MidiSendMessageResult::Other;
        }

        return midi2::MidiSendMessageResult::Succeeded;

    }



    _Use_decl_annotations_
    midi2::MidiSendMessageResult MidiEndpointConnection::SendMultipleMessagesWordList(
        internal::MidiTimestamp timestamp,
        collections::IVectorView<uint32_t> const& words) noexcept
    {
        internal::LogInfo(__FUNCTION__, L"Enter");

        uint32_t i{ 0 };

        uint32_t messageWords[4]{ 0,0,0,0 };   // we reuse this storage

        collections::IIterable iterable = words.as<collections::IIterable<uint32_t>>();

        auto iter = iterable.First();

        auto vectorSize = words.Size();

        while (iter.HasCurrent())
        {
            //auto messageWordCount = internal::GetUmpLengthInMidiWordsFromFirstWord(words.GetAt(i));
            auto messageWordCount = internal::GetUmpLengthInMidiWordsFromFirstWord(iter.Current());

            if (i + messageWordCount <= vectorSize)
            {
                for (uint32_t j = 0; j < messageWordCount; j++)
                {
                    //messageWords[j] = words.GetAt(i + j);

                    messageWords[j] = iter.Current();
                    iter.MoveNext();
                }

                auto sendMessageResult = SendMessageRaw(m_endpointAbstraction, (VOID*)(messageWords), messageWordCount * sizeof(uint32_t), timestamp);

                if (SendMessageFailed(sendMessageResult))
                {
                    // failed. Log and return. We fail on first problem.
                    internal::LogGeneralError(__FUNCTION__, L"Failed to send message");

                    if (i > 0)
                    {
                        sendMessageResult |= midi2::MidiSendMessageResult::MessageListPartiallyProcessed;
                    }

                    return sendMessageResult;
                }

                i += messageWordCount;
            }
        }

        return midi2::MidiSendMessageResult::Succeeded;

    }

    _Use_decl_annotations_
    midi2::MidiSendMessageResult MidiEndpointConnection::SendMultipleMessagesWordArray(
        internal::MidiTimestamp timestamp,
        winrt::array_view<uint32_t> words) noexcept
    {
        internal::LogInfo(__FUNCTION__, L"Enter");

        uint32_t i{ 0 };

        while (i < words.size())
        {
            auto messageWordCount = internal::GetUmpLengthInMidiWordsFromFirstWord(words[i]);

            if (i + messageWordCount <= words.size())
            {
                auto sendMessageResult = SendMessageRaw(m_endpointAbstraction, (VOID*)(words.data() + i), messageWordCount * sizeof(uint32_t), timestamp);

                if (SendMessageFailed(sendMessageResult))
                {
                    // failed. Log and return. We fail on first problem.
                    internal::LogGeneralError(__FUNCTION__, L"Failed to send message");

                    if (i > 0)
                    {
                        sendMessageResult |= midi2::MidiSendMessageResult::MessageListPartiallyProcessed;
                    }

                    return sendMessageResult;
                }

                i += messageWordCount;
            }
            else
            {
                // failed. Log and return. We fail on first problem.
                internal::LogGeneralError(__FUNCTION__, L"Failed to send message. Index out of bounds");

                return midi2::MidiSendMessageResult::Failed | midi2::MidiSendMessageResult::DataIndexOutOfRange;

            }
        }

        return midi2::MidiSendMessageResult::Succeeded;
    }



    _Use_decl_annotations_
    midi2::MidiSendMessageResult MidiEndpointConnection::SendMultipleMessagesStructList(
        internal::MidiTimestamp timestamp,
        collections::IVectorView<MidiMessageStruct> const& messages) noexcept
    {
        internal::LogInfo(__FUNCTION__, L"Enter");

        for (auto const& message : messages)
        {
            auto messageWordCount = internal::GetUmpLengthInMidiWordsFromFirstWord(message.Word0);
            auto sendMessageResult = SendMessageRaw(m_endpointAbstraction, (VOID*)&message, messageWordCount * sizeof(uint32_t), timestamp);

            if (SendMessageFailed(sendMessageResult))
            {
                // failed. Log and return. We fail on first problem.
                internal::LogGeneralError(__FUNCTION__, L"Failed to send message");

                return sendMessageResult;
            }
        }

        return midi2::MidiSendMessageResult::Succeeded;
    }



    _Use_decl_annotations_
    midi2::MidiSendMessageResult MidiEndpointConnection::SendMultipleMessagesStructArray(
        internal::MidiTimestamp timestamp,
        winrt::array_view<MidiMessageStruct> messages) noexcept
    {
        internal::LogInfo(__FUNCTION__, L"Enter");

        for (auto const& message : messages)
        {
            auto messageWordCount = internal::GetUmpLengthInMidiWordsFromFirstWord(message.Word0);
            auto sendMessageResult = SendMessageRaw(m_endpointAbstraction, (VOID*)&message, messageWordCount * sizeof(uint32_t), timestamp);

            if (SendMessageFailed(sendMessageResult))
            {
                // failed. Log and return. We fail on first problem.
                internal::LogGeneralError(__FUNCTION__, L"Failed to send message");

                return sendMessageResult;
            }
        }

        return midi2::MidiSendMessageResult::Succeeded;

    }


    _Use_decl_annotations_
    midi2::MidiSendMessageResult MidiEndpointConnection::SendMultipleMessagesPacketList(
        collections::IVectorView<IMidiUniversalPacket> const& messages) noexcept
    {
        internal::LogInfo(__FUNCTION__, L"Sending multiple message packet list");

        for (auto const& ump : messages)
        {
            auto result = SendUmpInternal(m_endpointAbstraction, ump);

            if (!MidiEndpointConnection::SendMessageSucceeded(result))
            {
                // if any fail, we return immediately.

                return result;
            }
        }

        return midi2::MidiSendMessageResult::Succeeded;

    }

    //_Use_decl_annotations_
    //midi2::MidiSendMessageResult MidiEndpointConnection::SendMultipleMessagesPacketArray(
    //    winrt::array_view<IMidiUniversalPacket> messages) noexcept
    //{
    //    internal::LogInfo(__FUNCTION__, L"Enter");

    //    for (auto const& ump : messages)
    //    {
    //        auto result = SendUmpInternal(m_endpointAbstraction, ump);

    //        if (!MidiEndpointConnection::SendMessageSucceeded(result))
    //        {
    //            // if any fail, we return immediately.

    //            return result;
    //        }
    //    }

    //    return midi2::MidiSendMessageResult::Succeeded;

    //}




}