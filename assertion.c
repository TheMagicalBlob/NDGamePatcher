#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#define AssertionMaxRecursionCount 0xA // The maximum depth for buffered assertion messages
#define AssertionMaxMessageSize 0x400



const char * AssertionMaxRecursionMsg = "MAX RECURSION EXCEEDED";

unsigned char AssertionMessageCurrentDepth = 0;
char AssertionMessageBuffer[AssertionMaxMessageSize * AssertionMaxRecursionCount] = {0};








__always_inline
const char * AssertPushBufferedMsg(const char * FormatString, ...)
{
    va_list list;
    va_start(list, FormatString);
    
    if (AssertionMessageCurrentDepth > AssertionMaxRecursionCount)
    {
        return AssertionMaxRecursionMsg;
    }

    char * newMessageOffset;
    newMessageOffset = AssertionMessageBuffer + (AssertionMessageCurrentDepth * AssertionMaxMessageSize);
    ++AssertionMessageCurrentDepth;

    vsnprintf(newMessageOffset, AssertionMaxMessageSize - 1, FormatString, list);

    va_end(list);
    return newMessageOffset;
}


__always_inline
const char * AssertPopBufferedMsg()
{
    const char * offset;
    --AssertionMessageCurrentDepth;

    if (AssertionMessageCurrentDepth <= AssertionMaxRecursionCount)
    {
        offset = (const char *) AssertionMessageBuffer + (AssertionMessageCurrentDepth * AssertionMaxMessageSize);
    }
    else {
        offset = AssertionMaxRecursionMsg;
    }

    return offset;
}




int echo(const char * string, ...);
void AssertPrint(const char * Cause, const char * File, const char * Function, const unsigned int Line, const char * Message)
{
    char output[0x800] = {0};
    char errMsg[0x80] = {0};
    const char * pErrMsg;
    pErrMsg = NULL;
      
    // Get buffered message
    if (Message == 0) {
        Message = AssertPopBufferedMsg();
    }

    if (errno)
    {
        snprintf(errMsg, sizeof(errMsg), "\n***   ERRNO:   %d->%s", errno, strerror(errno));
        pErrMsg = (const char *) errMsg;
    }

    if (pErrMsg == 0) {
        pErrMsg = "";
    }

    snprintf(output, 0x800,
        "\n"
        "******************************************************************************\n"
        "*** ASSERTION: %s%s\n"
        "***   Message: %s\n"
        "***\n"
        "***  Function: %s\n"
        "***      File: %s\n"
        "***      Line: %d\n"
        "***     Built: %s\n"
        "******************************************************************************\n",
        Cause, pErrMsg, Message, Function, File, Line, __DATE__ " @ " __TIME__
    );

    echo("%s", (char *) output);
}
