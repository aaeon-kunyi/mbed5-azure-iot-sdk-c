// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/consolelogger.h"

#ifndef NO_LOGGING

#ifdef WIN32
#include "windows.h"
#endif // WIN32


#ifdef WINCE
#include <stdarg.h>

void consolelogger_log(LOG_CATEGORY log_category, const char* file, const char* func, int line, unsigned int options, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    time_t t = time(NULL);

    switch (log_category)
    {
    case AZ_LOG_INFO:
        (void)printf("Info: ");
        break;
    case AZ_LOG_ERROR:
        (void)printf("Error: Time:%.24s File:%s Func:%s Line:%d ", ctime(&t), xlogging_get_filename(file), func, line);
        break;
    default:
        break;
    }

    (void)vprintf(format, args);
    va_end(args);

    (void)log_category;
    if (options & LOG_LINE)
    {
        (void)printf("\r\n");
    }
}
#endif // WINCE

LOGGER_LOG global_log_function = consolelogger_log;

void xlogging_set_log_function(LOGGER_LOG log_function)
{
    global_log_function = log_function;
}

LOGGER_LOG xlogging_get_log_function(void)
{
    return global_log_function;
}

const char* xlogging_get_filename(const char* fullpath)
{
    if (fullpath == NULL || fullpath[0] == 0)
    {
        return fullpath;
    }
    int len = strlen(fullpath);
    for (int i = len - 1; i >= 0; i--)
    {
        if (fullpath[i] == '/' || fullpath[i] == '\\')
        {
            return &fullpath[i + 1];
        }
    }
    return fullpath;
}

#if (defined(_MSC_VER)) && (!(defined WINCE))

LOGGER_LOG_GETLASTERROR global_log_function_GetLastError = consolelogger_log_with_GetLastError;

void xlogging_set_log_function_GetLastError(LOGGER_LOG_GETLASTERROR log_function_GetLastError)
{
    global_log_function_GetLastError = log_function_GetLastError;
}

LOGGER_LOG_GETLASTERROR xlogging_get_log_function_GetLastError(void)
{
    return global_log_function_GetLastError;
}
#endif

/* Print up to 16 bytes per line. */
#define LINE_SIZE 16

/* Return the printable char for the provided value. */
#define PRINTABLE(c)         ((c >= ' ') && (c <= '~')) ? (char)c : '.'

/* Convert the lower nibble of the provided byte to a hexadecimal printable char. */
#define HEX_STR(c)           (((c) & 0xF) < 0xA) ? (char)(((c) & 0xF) + '0') : (char)(((c) & 0xF) - 0xA + 'A')

void LogBinary(const char* comment, const void* data, size_t size)
{
    char charBuf[LINE_SIZE + 1];
    char hexBuf[LINE_SIZE * 3 + 1];
    size_t countbuf = 0;
    size_t i = 0;
    const unsigned char* bufAsChar = (const unsigned char*)data;
    const unsigned char* startPos = bufAsChar;

    LOG(AZ_LOG_TRACE, LOG_LINE, "%s     %lu bytes", comment, (unsigned long)size);

    /* Print the whole buffer. */
    for (i = 0; i < size; i++)
    {
        /* Store the printable value of the char in the charBuf to print. */
        charBuf[countbuf] = PRINTABLE(*bufAsChar);

        /* Convert the high nibble to a printable hexadecimal value. */
        hexBuf[countbuf * 3] = HEX_STR(*bufAsChar >> 4);

        /* Convert the low nibble to a printable hexadecimal value. */
        hexBuf[countbuf * 3 + 1] = HEX_STR(*bufAsChar);

        hexBuf[countbuf * 3 + 2] = ' ';

        countbuf++;
        bufAsChar++;
        /* If the line is full, print it to start another one. */
        if (countbuf == LINE_SIZE)
        {
            charBuf[countbuf] = '\0';
            hexBuf[countbuf * 3] = '\0';
            LOG(AZ_LOG_TRACE, LOG_LINE, "%p: %s    %s", startPos, hexBuf, charBuf);
            countbuf = 0;
            startPos = bufAsChar;
        }
    }

    /* If the last line does not fit the line size. */
    if (countbuf > 0)
    {
        /* Close the charBuf string. */
        charBuf[countbuf] = '\0';

        /* Fill the hexBuf with spaces to keep the charBuf alignment. */
        while ((countbuf++) < LINE_SIZE - 1)
        {
            hexBuf[countbuf * 3] = ' ';
            hexBuf[countbuf * 3 + 1] = ' ';
            hexBuf[countbuf * 3 + 2] = ' ';
        }
        hexBuf[countbuf * 3] = '\0';

        /* Print the last line. */
        LOG(AZ_LOG_TRACE, LOG_LINE, "%p: %s    %s", startPos, hexBuf, charBuf);
    }
}

#ifdef WIN32

#ifdef WINCE
void xlogging_LogErrorWinHTTPWithGetLastErrorAsStringFormatter(int errorMessageID)
{
    (void)errorMessageID;
}
#else // WINCE
void xlogging_LogErrorWinHTTPWithGetLastErrorAsStringFormatter(int errorMessageID)
{
    char messageBuffer[MESSAGE_BUFFER_SIZE];
    if (errorMessageID == 0)
    {
        LogError("GetLastError() returned 0. Make sure you are calling this right after the code that failed. ");
    }
    else
    {
        int size = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
            GetModuleHandle("WinHttp"), errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), messageBuffer, MESSAGE_BUFFER_SIZE, NULL);
        if (size == 0)
        {
            size = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), messageBuffer, MESSAGE_BUFFER_SIZE, NULL);
            if (size == 0)
            {
                LogError("GetLastError Code: %d. ", errorMessageID);
            }
            else
            {
                LogError("GetLastError: %s.", messageBuffer);
            }
        }
        else
        {
            LogError("GetLastError: %s.", messageBuffer);
        }
    }
}
#endif // WINCE

#endif // WIN32


#endif // NO_LOGGING


