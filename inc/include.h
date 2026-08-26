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
// #pragma once
typedef enum : char
{
    Invalid = -1, // Unknown or unsupported
    T2R,
    T1x,
    UC4,
    TLL

} game_id_t;




typedef enum wp_type
{
    WP_CODE, // Break on instruction execution only.
    WP_DATARW, // Break on data writes only.
    WP_IOWR, // Break on I/O reads or writes.
    WP_DATARW_NI  // Break on data reads or writes but not instruction fetches.

} wp_type_t;




typedef enum btree_type
{
    // interior_index = 0x2,
    interior_index = 0x0, // docs say 2, but we see 0 in it's place
    interior_table = 0x5,
    leaf_index = 0xA,
    leaf_table = 0xD

} btree_type_t;








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
    int status;
    game_id_t game_id;
    char title_id[0xA];
    uint32_t app_id;
    dmenu_menu_base_t * dev_menu_base;
    dmenu_menu_base_t * quick_menu_base;
    dmenu_menu_base_t * favorites_menu_base;


    // Addresses of various functions/variables in the game process
    int8_t * dat_novis;
    void * func_malloc;
    void * func_free;
    void * func_lookup_symbol;

} ndgame_t;




// NOTE: this database is big-endian, not little
// Typedef for sql-lite header format used to load the ps5's database files (app.db, in our case)
// 
//! TODO: These member names are a bit verbose, fix them
typedef struct
{
    char magic[0x10]; // The header string: Must be "SQLite format 3\000"
    short block_size; // The database page size in bytes. Must be a power of two between 512 and 32768 inclusive, or the value 1 representing a page size of 65536.
    char write_fformat; // File format write version. 1 for legacy; 2 for WAL.
    char read_fformat; // File format read version. 1 for legacy; 2 for WAL.
    char reserved_per_page; // Bytes of unused "reserved" space at the end of each page. Usually 0. 
    char max_payload_frac; // Maximum embedded payload fraction. Must be 64. 
    char min_payload_frac; // Minimum embedded payload fraction. Must be 32. 
    char leaf_payload_frac; // Leaf payload fraction. Must be 32.
    u_int file_change_count; // File change counter.
    u_int database_size; // Size of the database file in pages. The "in-header database size".;
    u_int first_freelist_trunk_page_number; // Page number of the first freelist trunk page.
    u_int freelist_page_count; // Total number of freelist pages.
    u_int schema_cookie; // The schema cookie.
    u_int schema_format_num; // The schema format number. Supported schema formats are 1, 2, 3, and 4.
    u_int page_cache_size_default; // Default page cache size.
    u_int largest_root_btree_number; // The page number of the largest root b-tree page when in auto-vacuum or incremental-vacuum modes, or zero otherwise.
    u_int db_text_encoding; // 	The database text encoding. A value of 1 means UTF-8. A value of 2 means UTF-16le. A value of 3 means UTF-16be.
    u_int user_version; // The "user version" as read and set by the user_version pragma.
    u_int inc_vaccum_mode; // True (non-zero) for incremental-vacuum mode. False (zero) otherwise.
    u_int app_id; // The "Application ID" set by PRAGMA application_id.
    const char reserved[0x14]; // Reserved for expansion. Must be zero.
    u_int version_valid_for; // The value of the change counter when the version number was stored. Indicates which transaction the version number is valid for.
    u_int SQLITE_VERSION_NUMBER; // Exactly what it says on the can

} database_t;



typedef struct 
{
    btree_type_t page_type; // the type of this page (interior table/index, leaf table/index)
    short first_freeblock_offset; // Gives the start of the first freeblock on the page, or is zero if there are no freeblocks. 
    short cell_num; // Gives the number of cells on the page.
    short cell_area_offset; // Start of the cell content area. A zero value is interpreted as 65536. 
    char frag_bytes_free; // Gives the number of fragmented free bytes within the cell content area.
    unsigned int int_btree_rm_ptr; // Right-most pointer; this value appears only in the header of interior b-tree pages, and is omitted from all other pages.

} database_btree_header_t;






int echo(const char * message, ...);
void log_message(const char * base_string, ...);
void notify(const char * message, ...);

char * strbufbuild(char * format, ...);

int get_longest_line_length(const char * str, size_t str_maxlen);

ndgame_t * get_nd_game();

int * get_watchpoint_count();

pid_t find_pid(const char * process_name);

