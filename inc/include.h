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
#define TESTS
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
    errno = 1;\
    if (errno) {\
        snprintf(output, sizeof(output), "%s\n# Message (errno): %s", output, strerror(errno));\
    }\
    unsigned int len;\
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
//## typedefs
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




#pragma once
// DMenu component virtual pointers
typedef struct dmenu_types_s
{
    dmenu_vptr_t menu_base; // The base for each debug menu
    
    dmenu_vptr_t menu; // The menus themselves which contain individual items
    dmenu_vptr_t dyn_menu; // Menus with dynamic contents that're cleared and repopulated every time they're opened
    dmenu_vptr_t weapon_upgrades_menu;

    dmenu_vptr_t item_submenu;
    dmenu_vptr_t task_item_submenu;
    dmenu_vptr_t subnode_task_item_submenu;

    dmenu_vptr_t base_component; // 0x??
    dmenu_vptr_t separator_item; // 0xA8
    dmenu_vptr_t toggle_item; // 0xB8
    dmenu_vptr_t exec_item; // 0xC0
    dmenu_vptr_t alt_exec_item;
    dmenu_vptr_t string_item;
    dmenu_vptr_t string_list_item;
    dmenu_vptr_t decimal_item;
    dmenu_vptr_t task_history_item;
    dmenu_vptr_t task_item;
    dmenu_vptr_t user_item; // Entirely useless, I assume it's meant to be replaced by an actual username
    dmenu_vptr_t timer_item;
    dmenu_vptr_t checkpoint_item;
    dmenu_vptr_t float_item; // 0x158

} dmenu_types_t;




typedef struct dmenu_menu_base_s dmenu_menu_base_t;
// Addresses of various functions and variables in the game process, assuming aslr has been removed
typedef struct dmenu_addresses_s
{
    dmenu_menu_base_t * dev_menu_base;
    dmenu_menu_base_t * quick_menu_base;
    dmenu_menu_base_t * favorites_menu_base;

    // Function addresses
    struct {
        void * malloc; // Malloc
        void * free;   // Free
        void * lookup_symbol; // Lookup a script symbol via a provided SID
        
    } func;
    
    // Variable addresses
    struct {
        byte * novis; // Toggle usage of visibility info (culling)

    } dat;

} dmenu_addresses_t;




// Struct for handling game-specific addresses and other variables. Assigned after attaching
typedef struct ndgame
{
    pid_t pid;
    int status; // Status reported by waitpid the last time the process stopped
    game_id_t game_id;

    char title_id[0xA];
    char app_ver[0xB]; // String representation
    char title[0x80];

    dmenu_types_t vptrs; // vPtr's for DMenu component class vTables
    dmenu_addresses_t addrs;

} ndgame_t;













int echo(const char * message, ...);
void log_message(const char * base_string, ...);
void notify(const char * message, ...);

char * strbufbuild(char * format, ...);

int get_longest_line_length(const char * str, size_t str_maxlen);

int * get_watchpoint_count();

pid_t find_pid(const char * process_name);

