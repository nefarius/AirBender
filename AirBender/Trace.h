/*++

Module Name:

    Trace.h

Abstract:

    This module contains the local type definitions for the
    driver.

Environment:

    Windows User-Mode Driver Framework 2

--*/

//
// Device Interface GUID
// a775e97e-a41b-4bfc-868e-25be84643b62
//
DEFINE_GUID(GUID_DEVINTERFACE_AIRBENDER,
    0xa775e97e,0xa41b,0x4bfc,0x86,0x8e,0x25,0xbe,0x84,0x64,0x3b,0x62);

//
// Define the tracing flags.
//
// Tracing GUID - 66da618e-49c7-43cf-90a8-65e1c41ff859
//

#define WPP_CONTROL_GUIDS                                              \
    WPP_DEFINE_CONTROL_GUID(                                           \
        MyDriver1TraceGuid, (66da618e,49c7,43cf,90a8,65e1c41ff859), \
                                                                            \
        WPP_DEFINE_BIT(MYDRIVER_ALL_INFO)                              \
        WPP_DEFINE_BIT(TRACE_DRIVER)                                   \
        WPP_DEFINE_BIT(TRACE_DEVICE)                                   \
        WPP_DEFINE_BIT(TRACE_QUEUE)                                    \
        WPP_DEFINE_BIT(TRACE_INTERRUPT)                                \
        WPP_DEFINE_BIT(TRACE_BULKRWR)                                  \
        WPP_DEFINE_BIT(TRACE_DS3)                                      \
        WPP_DEFINE_BIT(TRACE_BLUETOOTH)                                \
        )                             

#define WPP_FLAG_LEVEL_LOGGER(flag, level)                                  \
    WPP_LEVEL_LOGGER(flag)

#define WPP_FLAG_LEVEL_ENABLED(flag, level)                                 \
    (WPP_LEVEL_ENABLED(flag) &&                                             \
     WPP_CONTROL(WPP_BIT_ ## flag).Level >= level)

#define WPP_LEVEL_FLAGS_LOGGER(lvl,flags) \
           WPP_LEVEL_LOGGER(flags)
               
#define WPP_LEVEL_FLAGS_ENABLED(lvl, flags) \
           (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level >= lvl)

//
// This comment block is scanned by the trace preprocessor to define our
// Trace function.
//
// begin_wpp config
// FUNC Trace{FLAG=MYDRIVER_ALL_INFO}(LEVEL, MSG, ...);
// FUNC TraceEvents(LEVEL, FLAGS, MSG, ...);
// end_wpp
//

