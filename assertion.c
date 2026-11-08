#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#define AssertionMaxRecursionCount 0xA // The maximum depth for buffered assertion messages
#define AssertionMaxMessageSize 0x400



const char * AssertionMaxRecursionMsg = "MAX RECURSION EXCEEDED";

unsigned char AssertionCurrMsgDepth = 0;
char AssertionMessageBuffer[AssertionMaxMessageSize * AssertionMaxRecursionCount] = {0};






// Increment the current assertion message depth, and print the provided message to the current spot (assuming one's available)
const char * AssertPushBufferedMsg(const char * FormatString, ...)
{
    va_list list;
    va_start(list, FormatString);
    
    if (AssertionCurrMsgDepth > AssertionMaxRecursionCount)
    {
        return AssertionMaxRecursionMsg;
    }

    char * newMessageOffset;
    newMessageOffset = AssertionMessageBuffer + (AssertionCurrMsgDepth * AssertionMaxMessageSize);
    ++AssertionCurrMsgDepth;

    memset(newMessageOffset, 0, AssertionMaxMessageSize);
    vsnprintf(newMessageOffset, AssertionMaxMessageSize - 1, FormatString, list);

    va_end(list);
    return newMessageOffset;
}


static const char * AssertPullBufferedMsg()
{
    const char * offset;
    --AssertionCurrMsgDepth;

    if (AssertionCurrMsgDepth <= AssertionMaxRecursionCount)
    {
        offset = (const char *) AssertionMessageBuffer + (AssertionCurrMsgDepth * AssertionMaxMessageSize);
    }
    else {
        offset = AssertionMaxRecursionMsg;
    }

    return offset;
}




int echo(const char * string, ...);
void AssertPrint(const char * Cause, const char * File, const char * Function, const unsigned int Line, const char * Message, ...)
{
    char message[0x300] = {"<none>"};
    char output[0x800] = {""};
    char errMsg[0x80] = {0};

    va_list list;
    va_start(list, Message);


    // Set default message value
    if (Message != NULL) {
        vsnprintf(message, sizeof(message), Message, list);
    }

    if (errno)
    {
        // Append message for current errno value to initial assertion cause 
        snprintf(errMsg, sizeof(errMsg), "\n***   ERRNO:   %d->%s", errno, strerror(errno));
    }


    snprintf(output, 0x800,
        "\n"
        "*************************************************************************\n"
        "*** ASSERTION: %s%s\n"
        "***   Message: %s\n"
        "***\n"
        "***  Function: %s\n"
        "***      Line: %d\n"
        "***      File: %s\n"
        "***     Built: %s\n"
        "*************************************************************************\n",
        Cause, (char *) errMsg, message, Function, Line, File, __DATE__ " @ " __TIME__
    );

    echo("%s", (char *) output);
}
