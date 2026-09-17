#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/syscall.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/sysctl.h>
#include <sys/stat.h>

#include <ps5/klog.h>
#include <ps5/kernel.h>



//#
//## Global/macro definitiond
//#

#define null 0 // C# null
#define nullptr NULL
#define nullstr (char *) 0

#define true  (char) 1
#define false (char) 0
#define TRUE true
#define FALSE false

#define _SHELLCORE_AUTHID 0x4800000000010003L


#define PAYLOAD_NAME "dmenu_payload.bin"

// Do some basic extra debugging processes, as well as prints
#define DEBUG
#define ECHO
#ifdef ECHO
// #define VERBOSEDEBUG
#define LOGOUTPUT
#endif
// #define TESTS
// #define NOTIFY

#define __ERROR_PRINT(message) {\
    char output[0x400],\
         bars[0x100],\
         buf[0x600];\
    \
    snprintf(output, sizeof(output), "# ERROR:\n# File %s\n# Func %s\n# ~Line %d", __FILE__, __FUNCTION__, __LINE__ - 1);\
    \
    if (message != (char *) 0) {\
        snprintf(output, sizeof(output), "%s\n# Message: %s", output, message);\
    }\
    if (errno) {\
        snprintf(output, sizeof(output), "%s\n# Message (errno): %s", output, strerror(errno));\
    }\
    size_t len;\
    len = get_longest_line_length(output, sizeof(output));\
    for (unsigned int i = 0; i < sizeof(bars) && i < len + 2; i++) {\
        *(bars + i) = '#';\
    }\
    *(bars + len + 2) = 0;\
    snprintf(buf, sizeof(buf), "\n%s\n%s\n%s\n", bars, output, bars);\
    echo("%s", (char *) buf);\
\
    errno = 0;\
}



// Output error details if check evaluates to true. Does not return or break.
// Additionally check errno and output it's message accordingly
#define __err(check, message) {\
    if (check){\
        __ERROR_PRINT(message)\
    }\
}\



// Return void if check evaluates to true W/ output
// Additionally check errno and output it's message accordingly
#define __rerr(check, message) {\
    if (check){\
        __ERROR_PRINT(message)\
        return;\
    }\
}\



// Return provided error_code if check evaluates to true W/ output and return value/code
// Additionally check errno and output it's message accordingly
#define __rerrv(check, message, error_code) {\
    if (check){\
        __ERROR_PRINT(message)\
        return error_code;\
    }\
}\


// Assertion macro
#define __ASSERTIF {\
    setup_assertion(__FUNCTION__, __LINE__);\
}









//#
//## typedefs, enums, and structs
//#
typedef char sbyte;

typedef unsigned char byte;

typedef unsigned long SID;

typedef void * dmenu_vptr_t;



// Basic ID system for the loaded games. 
typedef enum game_id : char
{
    NONE =  0, // Unset
    T2   = -1, // The Last of Us Part II
    T2R  =  1, // The Last of Us Part II Remastered
    T1R  = -2, // The Last of Us Remastered
    T1X  =  2, // The Last of Us Remake/Part I
    UC4  = -3, // Uncharted 4
    UC4R =  3, // Uncharted 4 Remastered
    TLL  = -4, // Uncharted: The Lost Legacy
    TLLR =  4  // Uncharted: The Lost Legacy Remastered

} game_id_t;





typedef enum wp_type
{
    WP_CODE,     // Break on instruction execution only.
    WP_DATARW,   // Break on data writes only.
    WP_IOWR,     // Break on I/O reads or writes.
    WP_DATARW_NI // Break on data reads or writes but not instruction fetches.

} wp_type_t;




// Encoded title id strings for various checks
typedef enum title_id_check : SID
{
    // The Last of Us Part II Remastered
    PPSA10662 = 0x974EFE1763AEFE2A,
    PPSA15508 = 0x1710FB3F04A96B36,
    PPSA15511 = 0x1714083F04ABB724,
    PPSA15512 = 0x17140B3F04ABBC3D,

    // The Last of Us Part II
    CUSA10249 = 0x18B70D569D70EC85,
    CUSA07820 = 0xF9304A94558B97F6,
    CUSA13986 = 0xF98A0D6E7E8EC3E2,
    CUSA14006 = 0x715364774ECAA096,

    // The Last of Us Remake/Part I
    PPSA03396 = 0x69468F31BEE0F162,
    PPSA07642 = 0xBD192E52FE854BE4,
    PPSA07643 = 0xBD192F52FE854D97,

    // The Last of Us Remastered
    CUSA00552 = 0x175F978B48AA6A1B,
    CUSA00554 = 0x175F9D8B48AA744D,
    CUSA00556 = 0x175F9B8B48AA70E7,
    CUSA00557 = 0x175F9A8B48AA6F34,
    CUSA00559 = 0x175F908B48AA5E36

} title_id_check_t;




// Encoded app version strings for various checks
typedef enum app_version_sid : SID
{
    Vunknown = 0,
    V02001010 = 0x5E8413A7C3686AD1, // 02.001.010
    V02002000 = 0x682BA8AB0754E94D  // 02.002.000

} app_version_sid_t;










typedef struct notify_request
{
    char unused[45];
    char message[3075];

} notify_request_t;




typedef struct app_info
{
    uint32_t app_id;
    uint64_t unknown1;
    char     title_id[14];
    char     unknown2[0x3c];

} app_info_t;











int echo(const char * message, ...);
void log_message(const char * base_string, ...);
void notify(const char * message, ...);

char * strbufbuild(char * format, ...);

size_t get_longest_line_length(const char * str, size_t str_maxlen);

int * get_watchpoint_count();

pid_t find_pid(const char * process_name);

