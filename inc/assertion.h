#include <stdarg.h>

//#
//## Assertion-Related Functions
//#

int AssertPushBufferedMsg(const char * FormatString, ...);
const char * AssertClearBufferedMsg();

void AssertPrint(const char * Cause, const char * File, const char * Function, const unsigned int Line, const char * Message, ...);






//#
//## Assertion-Related Macro definitions
//#

// Output error details if check evaluates to true. Does not return or break.
// Additionally check errno and output it's message accordingly
#define __err(condition, message, ...) {\
    if (condition){\
        AssertPrint(#condition, __FILE__, __FUNCTION__, __LINE__ - 2, message, ##__VA_ARGS__);\
    }\
}\

// Output error details if check evaluates to true, then jump to the specified label
// Additionally check errno and output it's message accordingly
#define __errg(condition, label, message, ...) {\
    if (condition){\
        AssertPrint(#condition, __FILE__, __FUNCTION__, __LINE__ - 2, message, ##__VA_ARGS__);\
        goto label;\
    }\
}\



// Return void if check evaluates to true W/ output
// Additionally check errno and output it's message accordingly
#define __rerr(condition, message, ...) {\
    if (condition){\
        AssertPrint(#condition, __FILE__, __FUNCTION__, __LINE__ - 2, message, ##__VA_ARGS__);\
        return;\
    }\
}\



// Return provided error_code if check evaluates to true W/ output and return value/code
// Additionally check errno and output it's message accordingly
#define __rerrv(condition, return_value, message, ...) {\
    if (condition){\
        AssertPrint(#condition, __FILE__, __FUNCTION__, __LINE__ - 2, message, ##__VA_ARGS__);\
        return return_value;\
    }\
}\
