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


#define null NULL
#define nullptr (void *) 0
#define true 1
#define false 0
#define TRUE true
#define FALSE false



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


typedef struct dmenu_menu_base dmenu_menu_base_t;

#pragma once
// Structure for handling common app-specific variables //! re-write desc once sober
typedef struct nd_game
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



int echo(const char * message, ...);

void log_message(const char * base_string, ...);

void notify(const char * message, ...);

const ndgame_t * get_nd_game();
int * get_watchpoint_count();





