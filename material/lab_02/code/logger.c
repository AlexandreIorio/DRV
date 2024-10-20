#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include "logger.h"

// Define to 1 to enable logging
#define USE_LOG 1

// control structure for the logger
static struct {
	LogLevel currentLogLevel;
	bool logToConsole;
} logger_ctl = { ERROR, false };

const char *logLevelToString(LogLevel level)
{
	switch (level) {
	case DEBUG:
		return "DEBUG";
	case INFO:
		return "INFO";
	case WARNING:
		return "WARNING";
	case ERROR:
		return "ERROR";
	default:
		return "";
	}
}

const char *logLevelToColor(LogLevel level)
{
	switch (level) {
	case DEBUG:
		return DEBUG_COLOR;
	case INFO:
		return INFO_COLOR;
	case WARNING:
		return WARNING_COLOR;
	case ERROR:
		return ERROR_COLOR;
	default:
		return RESET_COLOR;
	}
}

void logMessage(LogLevel level, const char *format, ...)
{
#if USE_LOG
	if (level < logger_ctl.currentLogLevel) {
		return; // Skip logging if the level is below the current log level
	}

	// Get current time
	time_t now = time(NULL);
	char timeStr[20];
	strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S",
		 localtime(&now));

	// Format the log message
	char logBuffer[256];
	va_list args;
	va_start(args, format);
	vsnprintf(logBuffer, sizeof(logBuffer), format, args);
	va_end(args);

	// Write to log file
	FILE *logFile = fopen("log.txt", "a");
	if (logFile == NULL) {
		perror("Unable to open log file");
		return;
	}
	fprintf(logFile, "[%s] [%s]: %s\n", timeStr, logLevelToString(level),
		logBuffer);
	fclose(logFile);

	// Optionally print to console with colors
	if (logger_ctl.logToConsole) {
		printf("%s[%s] [%s]: %s%s\n", logLevelToColor(level), timeStr,
		       logLevelToString(level), logBuffer, RESET_COLOR);
	}
#endif
}

void setLogLevel(LogLevel level)
{
	logger_ctl.currentLogLevel = level;
	printf("Log level set to %s for pid n°: [%d]\n",
	       logLevelToString(level), getpid());
}

void enableConsoleLogging(bool enable)
{
	logger_ctl.logToConsole = enable;
}

void logger_help_args(const char *programName)
{
	printf("Logger usage:\n");
	printf("  -v, --verbosity LEVEL  Set the log level (DEBUG, INFO, WARNING, ERROR)\n");
	printf("  -h, --help             Show this help message and exit\n");
}

bool logger_process_args(int argc, char *argv[])
{
    LogLevel logLevelSet = ERROR;
    bool logLevelSpecified = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--logger-verbosity") == 0 || strcmp(argv[i], "-lv") == 0) {
            if (i + 1 < argc) {
                if (strcmp(argv[i + 1], "d") == 0) {
                    logLevelSet = DEBUG;
                } else if (strcmp(argv[i + 1], "i") == 0) {
                    logLevelSet = INFO;
                } else if (strcmp(argv[i + 1], "w") == 0) {
                    logLevelSet = WARNING;
                } else if (strcmp(argv[i + 1], "e") == 0) {
                    logLevelSet = ERROR;
                } else {
                    printf("Invalid log level: %s\n", argv[i + 1]);
                    logger_help_args(argv[0]);
                    return false;
                }
                logLevelSpecified = true;
                i++; // Sauter l'argument de niveau de log
            } else {
                logMessage(ERROR, "Missing log level after %s\n", argv[i]);
                logger_help_args(argv[0]);
                return false;
            }
        } else if (strcmp(argv[i], "--logger-console") == 0 || strcmp(argv[i], "-lc") == 0) {
            enableConsoleLogging(true);
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            logger_help_args(argv[0]);
            return false;
        } else {
            printf("Unknown argument: %s\n", argv[i]);
            logger_help_args(argv[0]);
            return false;
        }
    }

    if (logLevelSpecified) {
        setLogLevel(logLevelSet);
    }
    return true;
}
