﻿using System;
using System.Collections.Generic;
using System.Linq;

namespace SokkaServer.Children.DualShock3
{
    public enum DualShock3Buttons
    {
        Select = 1 << 0,
        LeftThumb = 1 << 1,
        RightThumb = 1 << 2,
        Start = 1 << 3,
        DPadUp = 1 << 4,
        DPadRight = 1 << 5,
        DPadDown = 1 << 6,
        DPadLeft = 1 << 7,
        LeftTrigger = 1 << 8,
        RightTrigger = 1 << 9,
        LeftShoulder = 1 << 10,
        RightShoulder = 1 << 11,
        Triangle = 1 << 12,
        Circle = 1 << 13,
        Cross = 1 << 14,
        Square = 1 << 15,
        Ps = 1 << 16
    }

    public class DualShock3InputReport
    {
        public DualShock3InputReport(byte[] buffer)
        {
            Buffer = buffer;
        }

        public byte[] Buffer { get; }

        public IEnumerable<DualShock3Buttons> EngagedButtons
        {
            get
            {
                var buttons =
                    (uint) ((Buffer[2] << 0) | (Buffer[3] << 8) | (Buffer[4] << 16) | (Buffer[5] << 24));

                return Enum.GetValues(typeof(DualShock3Buttons)).Cast<DualShock3Buttons>()
                    .Where(button => (buttons & (uint) button) == (uint) button);
            }
        }
    }
}