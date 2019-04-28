#ifndef __TRACE_H__ 
#define __TRACE_H__


// ConsoleApplicationTraceGuid GUID provider: {84bdb2e9-829e-41b3-b891-02f454bc2bd7}
#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID( \
        ConsoleApplicationTraceGuid, (84bdb2e9,829e,41b3,b891,02f454bc2bd7),  \
            WPP_DEFINE_BIT(TRACE_FLAG_CONSOLE_APP)        /* bit  0 = 0x00000001 */ \
    )

#define WPP_LEVEL_FLAGS_LOGGER(level, flags) WPP_LEVEL_LOGGER(flags)
#define WPP_LEVEL_FLAGS_ENABLED(level, flags) (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level >= level)

// begin_wpp config
//
// FUNC ConsoleAppLogTrace{LEVEL=TRACE_LEVEL_VERBOSE, FLAGS=TRACE_FLAG_CONSOLE_APP} (MSG, ...);
// FUNC ConsoleAppLogInfo{LEVEL=TRACE_LEVEL_INFORMATION, FLAGS=TRACE_FLAG_CONSOLE_APP}(MSG, ...);
// FUNC ConsoleAppLogWarning{LEVEL=TRACE_LEVEL_WARNING, FLAGS=TRACE_FLAG_CONSOLE_APP}(MSG, ...);
// FUNC ConsoleAppLogError{LEVEL=TRACE_LEVEL_ERROR, FLAGS=TRACE_FLAG_CONSOLE_APP}(MSG, ...);
// FUNC ConsoleAppLogCritical{LEVEL=TRACE_LEVEL_CRITICAL, FLAGS=TRACE_FLAG_CONSOLE_APP}(MSG, ...);
//
// end_wpp
//


#endif //__TRACE_H__