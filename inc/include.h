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

#define __ERROR_PRINT(check, message) {\
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
        *(bars + len + 2) = 0;\
\
        strncpy(buf, output, sizeof(output));\
        snprintf(buf, sizeof(output), "\n%s\n%s\n%s\n", bars, output, bars);\
        echo((char *) buf);\
    }\
}



// Output error details if check evaluates to true. Does not return or break.
// Additionally check errno and output it's message accordingly
#define __err(check, message) {\
    __ERROR_PRINT(check, message)\
}\



// Return void if check evaluates to true W/ output
// Additionally check errno and output it's message accordingly
#define __rerr(check, message) {\
    __ERROR_PRINT(check, message)\
    return;\
}\



// Return provided error_code if check evaluates to true W/ output and return value/code
// Additionally check errno and output it's message accordingly
#define __rerrv(check, message, error_code) {\
    __ERROR_PRINT(check, message)\
    return error_code;\
}\












// Basic ID system for the loaded games. 
typedef enum game_id : char
{
    NONE =  0, // Unset
    T2   = -1, // The Last of Us Part II
    T2R  =  1, // The Last of Us Part II Remastered
    T1R  = -2, // The Last of Us Remastered
    T1x  =  2, // The Last of Us Remake/Part I
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
typedef struct struct_dmenu_types_s dmenu_types_t;

// Struct for handling game-specific addresses and other variables. Assigned after attaching
typedef struct
{
    pid_t pid;
    game_id_t game_id;
    int status; // Status reported by waitpid the last time the process stopped
    int app_id;
    char app_ver[0xB];
    char title_id[0xA];
    char title[0x80];

    dmenu_menu_base_t * dev_menu_base;
    dmenu_menu_base_t * quick_menu_base;
    dmenu_menu_base_t * favorites_menu_base;

    const dmenu_types_t * vptrs;

    // Addresses of various functions/variables in the game process (not sure I'm actually gonna keep this, as it would be tedious to maintain)
    struct {
        // Boolean pointers
        struct {
            byte * novis; // Toggle usage of visibility info (culling)

        } bool;
    } dat;

    struct {
        void * malloc;
        void * free;
        void * lookup_symbol;
    } func;

} ndgame_t;













int echo(const char * message, ...);
void log_message(const char * base_string, ...);
void notify(const char * message, ...);

char * strbufbuild(char * format, ...);

int get_longest_line_length(const char * str, size_t str_maxlen);

int * get_watchpoint_count();

pid_t find_pid(const char * process_name);

