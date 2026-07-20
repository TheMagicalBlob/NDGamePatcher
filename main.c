/* Copyright (C) 2023 John Törnblom

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING. If not, see
<http://www.gnu.org/licenses/>.  */

#include "inc/memory.h"
#include "inc/dmenu.h"

#define MODULE_NAME "t2p.prx"
#define printf(s, ...) klog_printf(s, __VA_ARGS__)




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



typedef enum wp_type
{
    WP_CODE, // Break on instruction execution only.
    WP_DATARW, // Break on data writes only.
    WP_IOWR, // Break on I/O reads or writes.
    WP_DATARW_NI  // Break on data reads or writes but not instruction fetches.

} wp_type_t;




extern int sceKernelSendNotificationRequest(int, notify_request_t*, size_t, int);
extern int sceKernelGetAppInfo(pid_t pid, app_info_t *info);
// extern int sceKernelLoadStartModule(const char *name, uint64_t argc, const void *argv, uint32_t flags, void *, int *result);
// extern uint sceShellCoreUtilGetAppLaunchedParamString(int appid, const char * name, char * dst , size_t max);

// Not being used atm.
//! TODO: Reimplement once sigterm is being handled and we can actually close the file handles properly
static int logfile;

// The title id of the game being traced
const char _active_game_id[0xA];

nd_game_t nd_game;













int echo(const char * str, ...)
{
    int len;
    va_list list;
    va_start(list, str);
    
    char msg[4000];
    len = vsnprintf(msg, sizeof(msg) - 2, str, list);
    
    klog_printf("%s\n", msg);
    log_message("%s\n", msg);
    va_end(list);
    
    
    return len;
}


void notify(const char * message, ...)
{
    va_list list;
    notify_request_t request = { };

    // Build string with variable arguments
    va_start(list, message);
    vsnprintf(request.message, sizeof(request.message) - 1, message, list);
    va_end(list);
    
    sceKernelSendNotificationRequest(0, &request, sizeof(request), 0); // Not sure what the first and last arg are for. One may be notification type though
    
    
    printf("[Notification]: %s\n", request.message);
}


void log_message(const char * base_string, ...)
{
    va_list args;
    va_start(args, base_string);

    char message[4000];
    vsnprintf(message, sizeof(message), base_string, args);
    
    va_end(args);


    if (logfile < 0) { // failed to create it, don't wanna retry every print
        return;
    }

    if (!logfile)
    {
        logfile = open("/data/blog.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

        if (logfile < 0)
        {
            printf("Unable to open logfile, maybe error code: %s", strerror(logfile));
        }
    }


    // Write the message to the log file
    write(logfile, message, strlen(message));
}















// Wait for an eboot.bin process to be found, then connect and attach to it if it's title id matches one of the known ones
// Saves the pid, app id, and title id of the attached process to the provided nd_game struct
// 
//! TODO: Add support for the other naughtydog executable names so the payload works regardless of which is running
static int wait_for_game_launch(nd_game_t * nd_game)
{
    pid_t pid;
    int ret, attempts = 0;
    char * titleids[7] =
    {
        "PPSA10662",
        "PPSA15508",
        "PPSA15511",
        "PPSA15512",
        "PPSA03396",
        "PPSA07642",
        "PPSA07643",
    };

    pid_t skipped_ids[0x10];


    // Check for any running eboot.bin //! remember to check for the other possible T2R executable names
_retry:
    pid = find_pid("eboot.bin");

    if (pid <= 0)
    {
        echo("Unable to find eboot, waiting a few seconds before retrying");
        sleep(7);
        
        do {
            pid = find_pid("eboot.bin");
        }
        while (pid <= 0);


        // process started- wait 20 seconds for the game to finish launching
        sleep(20);
    }

    // Skip any pid's that have already been checked and fell through
    if (attempts)
    {
        for (int i = 0; i < attempts; i++)
        {
            if (pid == skipped_ids[i])
            {
                sleep(1);
                goto _retry;
            }
        }
    }
    
    echo("Found eboot.bin (pid %d), getting title id...", pid);
    
    
    app_info_t app_info;
    ret = sceKernelGetAppInfo(pid, &app_info);
    if (ret)
    {
        echo("Error getting app info, retrying in 5 seconds");
        sleep(5);
        goto _retry;
    }

    

    for (int i = 0; i < sizeof(titleids); i++)
    {
        if (!strncmp(app_info.title_id, titleids[i], 0xA))
        {
            int status;
            if ((status = set_dbg_perms()))
            {
                perror("set_dbg_perms(): ");
                echo("ERROR: unknown issue setting debug perms. (set_dbg_perms returned %d)", status);
                return 1;
            }
            
            if ((status = attach(pid)))
            {
                return 1;
            }


            echo("Saving app info...");
            nd_game->app_id = app_info.app_id;
            nd_game->pid = pid;
            strncpy(nd_game->title_id, app_info.title_id, 9);


            notify("Attached to game process.\nTitle ID: %s\nApp ID: %lX\nProcess ID: %d", nd_game->title_id, nd_game->app_id, pid);
            
            echo("Waiting for pid...");
            proc_wait(pid);
            echo("Process paused");
            return 0;
        }
    }


    echo("Application did not match any known title ids, retrying in 5 seconds");
    skipped_ids[attempts++] = pid;
    sleep(5);
    goto _retry;
}






static int find_and_trace_game()
{
    bzero(&nd_game, sizeof(nd_game_t));

    while (wait_for_game_launch(&nd_game))
    {
        echo("Error occured when waiting for game launch, waiting moment and retrying (10 seconds)");
        sleep(5);
        echo("5 seconds");
    }

    
    switch(*((unsigned long *) ((unsigned long) nd_game.title_id + 1)))
    {
        // T2R
        case 3618138931251467088: // PSA10662
        case 4048794567581979472: // PSA15508
        case 3544672884293194576: // PSA15511
        case 3616730478331122512: // PSA15512
            nd_game.game_id = T2R;
            nd_game.novis_address = T2R_NOVIS;
            nd_game.malloc = T2R_MALLOC;
            nd_game.free = T2R_FREE;
            load_menu_base(nd_game.pid, T2RDMENU_DEV_MENU_BASE_PTR, &nd_game.dev_menu_base);
            load_menu_base(nd_game.pid, T2RDMENU_QUICK_MENU_BASE_PTR, &nd_game.quick_menu_base);
            load_menu_base(nd_game.pid, T2RDMENU_FAVORITES_MENU_BASE_PTR, &nd_game.favorites_menu_base);
            break;

        // T1X
        case 3907210446666552144: // PSA03396
        case 3617576011346039632: // PSA07642 
        case 3689633605383967568: // PSA07643
            nd_game.game_id = T1x;
            nd_game.novis_address = T1x_NOVIS;
            nd_game.malloc = T1x_MALLOC;
            nd_game.free = T1x_FREE;
            load_menu_base(nd_game.pid, T1xDMENU_DEV_MENU_BASE_PTR, &nd_game.dev_menu_base);
            load_menu_base(nd_game.pid, T1xDMENU_QUICK_MENU_BASE_PTR, &nd_game.quick_menu_base);
            load_menu_base(nd_game.pid, T1xDMENU_FAVORITES_MENU_BASE_PTR, &nd_game.favorites_menu_base);
            break;


        default:
            bzero(&nd_game, sizeof(nd_game_t));
            echo("Unknown game title id read (%s -> %lld)", ((char *) ((unsigned long) nd_game.title_id + 1)), *((unsigned long *) ((unsigned long) nd_game.title_id + 1)));
            return 1;
    }

    return 0;
}










// Main payload loop (well, it will be a loop eventually)
static int start(int module_pid)
{
    int ret;
    ret = find_and_trace_game();
    if (ret)
    {
        echo("ERROR: Unable to load game");
    }


    echo("\n\n\n");
    dump_menu_entries(nd_game, &nd_game.dev_menu_base);
    echo("\n\n\n");
    dump_menu_entries(nd_game, &nd_game.quick_menu_base);
    echo("\n\n\n");
    // dump_menu_entries(nd_game, &nd_game.favorites_menu_base);
    goto _detach;


    // Make sure novis is present
    add_novis_option(&nd_game);
    
    dmenu_item_t item;
    load_item(nd_game.pid, find_item(nd_game.pid, &nd_game.quick_menu_base, "Manual Camera"), &item);
    ret = set_watchpoint(nd_game.pid, 0, item.event_handler, DBREG_DR7_EXEC, DBREG_DR7_LEN_1);
    if (ret)
    {
        echo("Error setting watchpoint, aborting.");
        goto _detach;
    }

    // Resume the process now that the breakpoint's set on all threads
    proc_continue(nd_game.pid, nullptr);
    proc_wait(nd_game.pid); // Wait for it to be hit


    _detach:
    return 0;
}





void print_exit_message()
{
    klog_printf("\n");
    klog_printf("========================\n");
    klog_printf("| DMenu Payload Exited |\n");
    klog_printf("========================\n\n");
}

void cleanup()
{
    if (*(char *)&nd_game)
    {
        echo("Clearing watchpoints and detaching from process");
        clear_watchpoint(nd_game.pid, -1);
        detach(nd_game.pid, __FUNCTION__);
    }

    if (logfile)
    {
        echo("Closing logfile handle");
        close(logfile);
    }

    exit(0);
}


void sighandler(int signal)
{
    if (signal == SIGSEGV)
    {
        echo("Segmentation fault received! Process exiting with error.");
    }
    else if (signal == SIGTERM)
    {
        echo("Process termiation signal received, process exiting without error.");
    }
    else {
        echo("Miscellaneous signal recieved, exiting (sig: %d)", signal);
    }

    cleanup();
}


int main()
{
    // Set thread name
    syscall(SYS_thr_set_name, -1, MODULE_NAME);
    
    // Set cleanup shit so I can stop rebooting the game every time the payload is killed
    struct sigaction handler;
    bzero(&handler, sizeof(struct sigaction));
    handler.__sigaction_u.__sa_handler = sighandler;
    
    atexit(print_exit_message);
    sigaction(SIGTERM, &handler, NULL);
    sigaction(SIGSEGV, &handler, NULL);
    


    // Kill any previous instances
    int ret, pid;
    while ((pid = find_pid(MODULE_NAME)) > 0)
    {
        printf("Killing old instance (pid: %d)...\n", pid);

        if ((ret = kill(pid, SIGTERM)))
        {
            perror("SIGKILL ERROR: ");
            break;
        }
        
        const char * msg = "Killed previous %s instance, exiting.";
        notify(msg, MODULE_NAME);
        echo(msg, MODULE_NAME);
        return 0;
    }
    


    // Payload start
    echo("Proceeding with new %s instance", MODULE_NAME);
    start(getpid());
    
    cleanup();
    return 0;
}

