#include "inc/include.h"

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


//! TODO: Give this the same output formatting as the error macro's output
void AssertionHandler(int signal, siginfo_t * info, void * vcontext)
{   
    ucontext_t * context = (ucontext_t *) vcontext;

    #if VERBOSITY > 1
    echo("pre-assertion context:\n# mc_addr: %llX\n# mc_rip: %llX\n# mc_flags: %llX", context->uc_mcontext.mc_addr, context->uc_mcontext.mc_rip, context->uc_mcontext.mc_flags);
    #endif

    if (signal == SIGSEGV)
    {
        char assertionMessage[0x200], registersMessage[0x200], bar[0x50];
        
        const char * assertionFormatString = "# Segmentation fault received.\n# \n# Reason:\n#";

        if (info->si_code == SEGV_MAPERR)
        {
            if (info->si_addr == nullptr)
            {
                snprintf(assertionMessage, sizeof(assertionMessage), "%s Null pointer", assertionFormatString);
            }
            else{
                snprintf(assertionMessage, sizeof(assertionMessage), "%s Address 0x%llX was invalid", assertionFormatString, (unsigned long long) info->si_addr);
            }
        }
        else if (info->si_code == SEGV_ACCERR)
        {
            snprintf(assertionMessage, sizeof(assertionMessage), "%s Access to 0x%llX is not permitted", assertionFormatString, (unsigned long long) info->si_addr);
        }
        else{
            snprintf(assertionMessage, sizeof(assertionMessage), "%s Unknown Cause %d; Cause address: 0x%llX", assertionFormatString, info->si_code, (unsigned long long) info->si_addr);
        }

        

        if (0) // Disabled for now, seems off (like RIP always being 1)
        snprintf(registersMessage, sizeof(registersMessage),
            "# Registers:\n"
            "# rax: %lX\n# rbx: %lX\n"
            "# rcx: %lX\n# rdx: %lX\n"
            "# rsi: %lX\n# rdi: %lX\n"
            "# rbp: %lX\n# rsp: %lX\n"
            "# r8:  %lX\n# r9:  %lX\n"
            "# r10: %lX\n# r11: %lX\n"
            "# r12: %lX\n# r13: %lX\n"
            "# r14: %lX\n# r15: %lX\n"
            "# rip: %lX\n# \n# eflags: %ld\n"
            "# trapno:  %d\n# err: %ld",

            context->uc_mcontext.mc_rax,    context->uc_mcontext.mc_rbx,
            context->uc_mcontext.mc_rcx,    context->uc_mcontext.mc_rdx,
            context->uc_mcontext.mc_rsi,    context->uc_mcontext.mc_rdi,
            context->uc_mcontext.mc_rbp,    context->uc_mcontext.mc_rsp,
            context->uc_mcontext.mc_r8,     context->uc_mcontext.mc_r9,
            context->uc_mcontext.mc_r10,    context->uc_mcontext.mc_r11,
            context->uc_mcontext.mc_r12,    context->uc_mcontext.mc_r13,
            context->uc_mcontext.mc_r14,    context->uc_mcontext.mc_r15,
            context->uc_mcontext.mc_rip,    context->uc_mcontext.mc_rflags,
            context->uc_mcontext.mc_trapno, context->uc_mcontext.mc_err
        );

        

        // Build enclosing '#' bars
        size_t len;
        len = get_longest_line_length(assertionMessage, sizeof(assertionMessage));
        if (len > 0x50) {
            len = 0x50;
        }
        for (unsigned i = 0; i < sizeof(bar) && i < len + 2; *(bar + i ++) = '#');
        *(bar + len + 2) = 0;

        echo("\n%s\n%s\n# \n%s\n", bar, assertionMessage, bar);
    }
    else if (signal == SIGTERM)
    {
        echo("Process termiation signal received, process exiting without error.");
    }
    else {
        echo("Miscellaneous signal recieved, exiting (sig: %d)", signal);
    }
}


