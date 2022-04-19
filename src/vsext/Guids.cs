// Guids.cs
// MUST match guids.h
using System;

namespace CodeRainbow
{
    static class GuidList
    {
        public const string guidcr_vsextPkgString = "56953d41-9f83-4875-94de-e32c1b20d868";
        public const string guidcr_vsextCmdSetString = "cc799d46-2ca1-436e-8531-6b9e4b7d8034";
        public const string guidToolWindowPersistanceString = "1f42023c-80d5-44ae-980c-ab189b1605ea";

        public static readonly Guid guidcr_vsextCmdSet = new Guid(guidcr_vsextCmdSetString);
    };
}