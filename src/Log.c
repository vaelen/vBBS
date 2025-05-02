/*
Copyright (c) 2025, Andrew Young

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
 
1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <vBBS/Types.h>

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <vBBS/Log.h>

const char *LEVELS[] = {
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR"
};

FILE *LOG;

/**
 * Initialize the log file. 
 * If the log file already exists, 
 * it will be closed and a new one will be opened.
 * @param filename The name of the log file to open.
 */
void InitLog(const char *filename)
{
    if (LOG != NULL)
    {
        CloseLog();
    }

    LOG = fopen(filename, "a");
    if (LOG == NULL)
    {
        fprintf(stderr, "Error opening log file: %s\n", filename);
        exit(1);
    }
}
/**
 * Close the log file if it is open.
 */
void CloseLog(void)
{
    if (LOG != NULL)
    {
        fclose(LOG);
        LOG = NULL;
    }
}

/** 
 * Helper method to log to a stream. 
 * This is called by other methods here. 
 */
void _Log(FILE *stream, LogLevel level, const char *format, va_list args)
{
    if (stream == NULL)
    {
        return;
    }

    fprintf(stream, "[%s] ", LEVELS[level]);
    vfprintf(stream, format, args);
    fprintf(stream, "\n");
}

/** 
 * Helper method to log to stderr and the log if it is defined. 
 * This is called by other methods. 
 */
void _LogMessage(LogLevel level, const char *format, va_list args)
{
    _Log(stderr, level, format, args);
    fflush(stderr);

    if (LOG != NULL)
    {
        _Log(LOG, level, format, args);
        fflush(LOG);
    }
}

/**
 * Log a message to the log file and stderr. 
 */
void LogMessage(LogLevel level, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    _LogMessage(level, format, args);
    va_end(args);
}

/**
 * Log a debug message to the log file and stderr. 
 */
void Debug(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    _LogMessage(LOG_DEBUG, format, args);
    va_end(args);
}

/**
 * Log an info message to the log file and stderr. 
 */
void Info(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    _LogMessage(LOG_INFO, format, args);
    va_end(args);
}

/**
 * Log a warning message to the log file and stderr. 
 */
void Warning(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    _LogMessage(LOG_WARNING, format, args);
    va_end(args);
}

/**
 * Log an error message to the log file and stderr. 
 */
void Error(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    _LogMessage(LOG_ERROR, format, args);
    va_end(args);
}
