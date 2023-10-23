﻿using Spectre.Console;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Midi2;

namespace Microsoft.Devices.Midi2.ConsoleApp
{
    internal class AnsiConsoleOutput
    {
        public static void DisplayMidiMessage(MidiMessageStruct ump, uint numWords, double offsetMicroseconds, UInt64 timestamp, uint index)
        {
            string data = string.Empty;

            string detailedMessageType = MidiMessageUtility.GetMessageFriendlyNameFromFirstWord(ump.Word0);

            string word0Text = string.Empty;
            string word1Text = string.Empty;
            string word2Text = string.Empty;
            string word3Text = string.Empty;


            if (numWords >= 4)
            {
                word3Text = string.Format("{0:X8}", ump.Word3);
            }

            if (numWords >= 3)
            {
                word2Text = string.Format("{0:X8}", ump.Word2);
            }

            if (numWords >= 2)
            {
                word1Text = string.Format("{0:X8}", ump.Word1);
            }

            word0Text = string.Format("{0:X8}", ump.Word0);

            const string offsetUnitMicroseconds = "μs";
            const string offsetUnitMilliseconds = "ms";
            const string offsetUnitSeconds = "s ";

            double offsetValue;
            string unitLabel;

            if (offsetMicroseconds > 1000000)
            {
                unitLabel = offsetUnitSeconds;
                offsetValue = offsetMicroseconds / 1000000;
            }
            else if (offsetMicroseconds > 1000)
            {
                unitLabel = offsetUnitMilliseconds;
                offsetValue = offsetMicroseconds / 1000;
            }
            else
            {
                unitLabel = offsetUnitMicroseconds;
                offsetValue = offsetMicroseconds;
            }

            string messageLineFormat =
                "[grey]{0,8}[/] \u2502 " +
                "[darkseagreen2]{1,19:N0}[/] \u2502 " +
                "[darkseagreen2]{2,10:F2}[/] " + unitLabel + " \u2502 " +
                "[deepskyblue1]{3,8}[/] [deepskyblue2]{4,8}[/] [deepskyblue3]{5,8}[/] [deepskyblue4]{6,8}[/] \u2502 " +
                "[steelblue1_1]{7,-20}[/]";


            AnsiConsole.MarkupLine(
                messageLineFormat,
                index,
                timestamp,
                offsetValue,
                word0Text, word1Text, word2Text, word3Text,
                detailedMessageType
                );


            //const string messageLineFormat =
            //    "[grey]{0,7}[/] | " +
            //    "[darkseagreen2]{1,12:F2}[/] µs | " +
            //    "[deepskyblue1]{2,8}[/] [deepskyblue2]{3,8}[/] [deepskyblue3]{4,8}[/] [deepskyblue4]{5,8}[/] | " +
            //    "[steelblue1_1]{6,-20}[/]";

            //AnsiConsole.MarkupLine(
            //    messageLineFormat, 
            //    index, 
            //    MidiClock.ConvertTimestampToMicroseconds(deltaTimestamp), 
            //    word0Text, word1Text, word2Text, word3Text,
            //    detailedMessageType
            //    ) ;


        }




    }
}