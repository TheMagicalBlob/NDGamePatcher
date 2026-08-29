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


#define null 0 // C# null
#define nullptr NULL
#define nullstr (char *) 0

typedef char sbyte;
typedef unsigned char byte;
#define true  (byte) 1
#define false (byte) 0
#define TRUE true
#define FALSE false

#define _SHELLCORE_AUTHID 0x4800000000010003L

#define PAYLOAD_NAME "dmenu_payload.bin"
#define DEBUG
// #define LOGOUTPUT
// #define NOTIFY



// Error status check/output
// For where additional functionality beyond printing and asserting is not needed.
// If check evaluates to true, assert and print message if it isn't null.
// Additionally check errno and output it's message accordingly
#define __err(check, message) {\
    if (check) {\
        char output[0x400],\
             bars[0x100],\
             buf[0x600];\
\
        snprintf(output, sizeof(output), "# ERROR:\n# File %s\n# Func %s\n# Line %d", __FILE__, __FUNCTION__, __LINE__ - 1);\
\
        if ((char *) message && *(char *) message != 0) {\
            snprintf(output, sizeof(output), "%s\n# Message: %s", output, message);\
        }\
        if (errno) {\
            snprintf(output, sizeof(output), "%s\n# Message (errno): %s", output, strerror(errno));\
        }\
\
        int len;\
        len = get_longest_line_length(output, sizeof(output));\
        for (int i = 0; i < sizeof(bars) && i < len + 2; i++) {\
            *(bars + i) = '#';\
        }\
\
        strncpy(buf, output, sizeof(output));\
        snprintf(buf, sizeof(output), "\n%s\n%s\n%s\n", bars, output, bars);\
        echo((char *) buf);\
    }\
}


// Error status check/output W/ return code.
// For where additional functionality beyond printing and asserting is not needed.
// If check evaluates to true, assert and print message if it isn't null.
// Additionally check errno and output it's message accordingly
#define __rerr(check, message, error_code) {\
    if (check) {\
        char output[0x400],\
             bars[0x100],\
             buf[0x600];\
\
        snprintf(output, sizeof(output), "# ERROR:\n# File %s\n# Func %s\n# Line %d", __FILE__, __FUNCTION__, __LINE__ - 1);\
\
        if ((char *) message && *(char *) message != 0) {\
            snprintf(output, sizeof(output), "%s\n# Message: %s", output, message);\
        }\
        if (errno) {\
            snprintf(output, sizeof(output), "%s\n# Message (errno): %s", output, strerror(errno));\
        }\
\
        int len;\
        len = get_longest_line_length(output, sizeof(output));\
        for (int i = 0; i < sizeof(bars) && i < len + 2; i++) {\
            *(bars + i) = '#';\
        }\
\
        strncpy(buf, output, sizeof(output));\
        snprintf(buf, sizeof(output), "\n%s\n%s\n%s\n", bars, output, bars);\
        echo((char *) buf);\
\
        return error_code;\
    }\
}












// Basic ID system for the loaded games. 
typedef enum game_identifier : char
{
    Invalid = -1, // Unknown or unsupported
    T2R,
    T1x,
    UC4,
    TLL

} game_id_t;




typedef enum watchpoint_type
{
    WP_CODE,     // Break on instruction execution only.
    WP_DATARW,   // Break on data writes only.
    WP_IOWR,     // Break on I/O reads or writes.
    WP_DATARW_NI // Break on data reads or writes but not instruction fetches.

} wp_type_t;












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




#pragma once
typedef struct dmenu_menu_base_s dmenu_menu_base_t;
// Struct for handling game-specific addresses and other variables. Assigned after attaching
typedef struct
{
    pid_t pid;
    game_id_t game_id;
    int status; // Status reported by waitpid the last time the process stopped
    int app_id;
    char app_ver[0xA];
    char title_id[0x80];
    char title[0x40];
    dmenu_menu_base_t * dev_menu_base;
    dmenu_menu_base_t * quick_menu_base;
    dmenu_menu_base_t * favorites_menu_base;


    // Addresses of various functions/variables in the game process
    byte * dat_novis;
    void * func_malloc;
    void * func_free;
    void * func_lookup_symbol;

} ndgame_t;












int echo(const char * message, ...);
void log_message(const char * base_string, ...);
void notify(const char * message, ...);

char * strbufbuild(char * format, ...);

int get_longest_line_length(const char * str, size_t str_maxlen);

ndgame_t * get_ndgame();

int * get_watchpoint_count();

pid_t find_pid(const char * process_name);

