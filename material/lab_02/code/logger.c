#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <stdbool.h>
#include "logger.h"

// control structure for the logger
static struct {
    LogLevel currentLogLevel;
    bool logToConsole;
} logger_ctl;

const char* logLevelToString(LogLevel level) {
    switch (level) {
        case DEBUG: return "DEBUG";
        case INFO: return "INFO";
        case WARNING: return "WARNING";
        case ERROR: return "ERROR";
        default: return "";
    }
}

const char* logLevelToColor(LogLevel level) {
    switch (level) {
        case DEBUG: return DEBUG_COLOR;
        case INFO: return INFO_COLOR;
        case WARNING: return WARNING_COLOR;
        case ERROR: return ERROR_COLOR;
        default: return RESET_COLOR;
    }
}

void logMessage(LogLevel level, const char* format, ...) {
    if (level < logger_ctl.currentLogLevel) {
        return; // Skip logging if the level is below the current log level
    }

    // Get current time
    time_t now = time(NULL);
    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&now));

    // Format the log message
    char logBuffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(logBuffer, sizeof(logBuffer), format, args);
    va_end(args);

    // Write to log file
    FILE* logFile = fopen("log.txt", "a");
    if (logFile == NULL) {
        perror("Unable to open log file");
        return;
    }
    fprintf(logFile, "[%s] [%s]: %s\n", timeStr, logLevelToString(level), logBuffer);
    fclose(logFile);

    // Optionally print to console with colors
    if (logger_ctl.logToConsole) {
        printf("%s[%s] [%s]: %s%s\n", logLevelToColor(level), timeStr, logLevelToString(level), logBuffer, RESET_COLOR);
    }
}

void setLogLevel(LogLevel level) {
    logger_ctl.currentLogLevel = level;
    printf("Log level set to %s for pid n°: [%d]\n", logLevelToString(level), getpid());
}

void enableConsoleLogging(bool enable) {
    logger_ctl.logToConsole = enable;
}