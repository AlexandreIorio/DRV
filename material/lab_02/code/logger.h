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

/// @brief This method is used to process the arguments of the program
/// @param argc the number of arguments
/// @param argv the vector arguments
/// @return true if the arguments are processed correctly, false otherwise
bool logger_process_args(int argc, char* argv[]);

/// @brief This method is used to display the help of the program
void logger_help_args(const char* programName);

#endif // LOGGER_H
