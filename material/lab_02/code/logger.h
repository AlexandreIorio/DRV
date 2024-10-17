#ifndef LOGGER_H
#define LOGGER_H

#include <stdbool.h>

// ANSI escape codes for colors
#define RESET_COLOR   "\x1b[0m"
#define DEBUG_COLOR   "\x1b[36m"  // Cyan
#define INFO_COLOR    "\x1b[32m"  // Vert
#define WARNING_COLOR "\x1b[33m"  // Jaune
#define ERROR_COLOR   "\x1b[31m"  // Rouge

// Log levels
typedef enum { DEBUG, INFO, WARNING, ERROR } LogLevel;

// control structure for the logger
static struct {
    LogLevel currentLogLevel;
    bool logToConsole;
} logger_ctl;


/// @brief Method to convert a log level to a string
const char* logLevelToString(LogLevel level);

/// @brief Method to convert a log level to a color
const char* logLevelToColor(LogLevel level);

/// @brief Method to log a message
void logMessage(LogLevel level, const char* format, ...);

/// @brief Method to set the log level
void setLogLevel(LogLevel level);

/// @brief Method to enable or disable console logging
void enableConsoleLogging(bool enable);

#endif // LOGGER_H
