#ifdef __cplusplus
extern "C" {
#endif
extern void logT(const char* fmt, ...);
extern void logD(const char* fmt, ...);
extern void logI(const char* fmt, ...);
extern void logW(const char* fmt, ...);
extern void logE(const char* fmt, ...);
extern void logMsg(const char* fmt, ...);
#ifdef __cplusplus
}
#endif

enum _logLevel {
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
};

#ifdef LOGGER_IMPLEMENTATION

#include <stdio.h>
#include <stdarg.h>

#define COLOR_RESET "\033[0m"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

static const char* _levelName(int level) {
    switch (level) {
        case LOG_TRACE: return "TRACE";
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO: return "INFO";
        case LOG_WARN: return "WARN";
        case LOG_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

#if defined(_WIN32) || defined(_WIN64) 
static WORD _levelColor(int level) {
    switch (level) {
        case LOG_TRACE: return FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        case LOG_DEBUG: return FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        case LOG_INFO: return FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        case LOG_WARN: return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        case LOG_ERROR: return FOREGROUND_RED | FOREGROUND_INTENSITY;
        default: return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    }
}
#else
static const char* _levelColor(int level) {
    switch (level) {
        case LOG_TRACE: return "\033[96m";
        case LOG_DEBUG: return "\033[94m";
        case LOG_INFO: return "\033[92m";
        case LOG_WARN: return "\033[93m";
        case LOG_ERROR: return "\033[91m";
        default: return "\033[37m";
    }
}
#endif

#if defined(_WIN32) || defined(_WIN64) 
static void _log(int level, const char* fmt, va_list args) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    WORD originalAttrs = csbi.wAttributes;
    WORD color = _levelColor(level);
    SetConsoleTextAttribute(hConsole, color);

    const char* name = _levelName(level);

    if (fmt == NULL || *fmt == '\0') {
        printf("[%s] %s\n", name, fmt);
    } else {
        printf("[%s] ", name);
        vprintf(fmt, args);
        printf("\n");
    }
    
    SetConsoleTextAttribute(hConsole, originalAttrs);
}
#else
static void _log(int level, const char* fmt, va_list args) {
    const char* color = _levelColor(level);
    const char* name = _levelName(level);

    if (fmt == NULL || *fmt == '\0') {
        printf("%s[%s] %s%s\n", color, name, fmt, COLOR_RESET);
    } else {
        printf("%s[%s] ", color, name);
        vprintf(fmt, args);
        printf("%s\n", COLOR_RESET);
    }
}
#endif

void logT(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    _log(LOG_TRACE, fmt, args);
    va_end(args);
}

void logD(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    _log(LOG_DEBUG, fmt, args);
    va_end(args);
}

void logI(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    _log(LOG_INFO, fmt, args);
    va_end(args);
}

void logW(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    _log(LOG_WARN, fmt, args);
    va_end(args);
}

void logE(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    _log(LOG_ERROR, fmt, args);
    va_end(args);
}

void logMsg(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    if (fmt == NULL || *fmt == '\0') {
        printf("%s\n", fmt);
    } else {
        vprintf(fmt, args);
        printf("\n");
    }
    va_end(args);
}

#endif //LOGGER_IMPLEMENTATION
