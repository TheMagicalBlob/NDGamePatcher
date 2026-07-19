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



extern uint sceShellCoreUtilGetAppLaunchedParamString(int appid, const char * name, char * dst , size_t max);

extern int sceKernelSendNotificationRequest(int, notify_request_t*, size_t, int);
extern int sceKernelGetAppInfo(pid_t pid, app_info_t *info);
// extern int sceKernelLoadStartModule(const char *name, uint64_t argc, const void *argv, uint32_t flags, void *, int *result);

// Not being used atm.
//! TODO: Reimplement once sigterm is being handled and we can actually close the file handles properly
static int logfile;

// The title id of the game being traced
const char _active_game_id[0xA];

nd_game_t game;



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
    return;
    
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





static pid_t wait_for_game_launch(const char * title_id)
{
    // Check for any running eboot.bin //! remember to check for the other possible T2R executable names
    pid_t pid = find_pid("eboot.bin");

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
    
    
    echo("Found eboot.bin (pid %d), getting title id...", pid);
    
    
    int ret;
    app_info_t app_info;
    ret = sceKernelGetAppInfo(pid, &app_info);
    if (ret)
    {
        echo("Error getting app info, off we fuck");
    }

    if (!strncmp(app_info.title_id, title_id, 0xA))
    {
        int status;

        if ((status = set_dbg_perms()) != 0)
        {
            return -1;
        }

        
        if ((status = attach(pid)) != 0)
        {
            return -1;
        }


        notify("Attached to game process.\nTitle ID: %s\nprocess id: %d", title_id, pid);
        
        echo("Waiting for pid...");
        proc_wait(pid);
        echo("Process paused");
        return pid;
    }
    else {
        notify("Application Title ID does not match expected value of [PPSA10662]\nTitle ID: %s\nApp ID: %d\n", app_info.title_id, app_info.app_id);
        return -1;
    }
}






static int load_game_by_title_id(const char * title_id)
{
    if (strlen(title_id) != 9)
    {
        echo("load_game_by_title_id(): ERROR, provided title_id was an invalid length");
    }
    bzero(&game, sizeof(nd_game_t));
    
    game.pid = wait_for_game_launch(title_id);
    
    switch(*(unsigned long *) title_id + 1)
    {
        // T2R
        case 3618138931251467088: // PSA10662
        case 4048794567581979472: // PSA15508
        case 3544672884293194576: // PSA15511
        case 3616730478331122512: // PSA15512
            game.game_id = T2R;
            game.novis_address = T2R_NOVIS;
            game.malloc = T2R_MALLOC;
            game.free = T2R_FREE;
            load_menu_base(game.pid, T2RDMENU_DEV_MENU_BASE_PTR, &game.dev_menu_base);
            load_menu_base(game.pid, T2RDMENU_QUICK_MENU_BASE_PTR, &game.quick_menu_base);
            load_menu_base(game.pid, T2RDMENU_FAVORITES_MENU_BASE_PTR, &game.favorites_menu_base);
            break;

        // T1X
        case 3907210446666552144: // PSA03396
        case 3617576011346039632: // PSA07642 
        case 3689633605383967568: // PSA07643
            game.game_id = T1x;
            game.novis_address = T1x_NOVIS;
            game.malloc = T1x_MALLOC;
            game.free = T1x_FREE;
            load_menu_base(game.pid, T1xDMENU_DEV_MENU_BASE_PTR, &game.dev_menu_base);
            load_menu_base(game.pid, T1xDMENU_QUICK_MENU_BASE_PTR, &game.quick_menu_base);
            load_menu_base(game.pid, T1xDMENU_FAVORITES_MENU_BASE_PTR, &game.favorites_menu_base);
            break;


        default:
            bzero(&game, sizeof(nd_game_t));
            echo("Unknown id %lld", *(unsigned long *) title_id + 1);
            return 1;
    }

    game.title_id = title_id;
    return 0;
}



static int start(int module_pid)
{
    const char * titleid = "PPSA10662";
    int ret;
    ret = load_game_by_title_id(titleid);
    if (ret)
    {
        echo("ERROR: Unable to load game with id %s", titleid);
    }
    
    // Make sure novis is present
    add_novis_option(&game);
    
    void * mancam_toggle_eh;
    dmenu_item_t item;
    load_item(game.pid, find_item(game.pid, &game.dev_menu_base, "Manual Camera"), &item);
    ret = set_watchpoint(game.pid, 0, mancam_toggle_eh, DBREG_DR7_EXEC, DBREG_DR7_LEN_1);
    if (ret)
    {
        echo("Error setting watchpoint, aborting.");
        goto _detach;
    }

    // Resume the process now that the breakpoint's set on all threads
    proc_continue(game.pid, nullptr);
    proc_wait(game.pid); // Wait for it to be hit


    _detach:
    return 0;
}



void cleanup(int signal)
{
    echo("Termination signal recieved, exiting (%d)", signal);

    if (*(char *)&game)
    {
        clear_watchpoint(game.pid, -1);
        detach(game.pid, __FUNCTION__);
    }

    if (logfile)
    {
        close(logfile);
    }

    exit(0);
}

int main()
{
    // Set thread name
    syscall(SYS_thr_set_name, -1, MODULE_NAME);
    
    // Set cleanup shit so I can stop rebooting the game every time the payload is killed
    struct sigaction handler;
    bzero(&handler, sizeof(struct sigaction));
    handler.__sigaction_u.__sa_handler = cleanup;
    
    sigaction(SIGTERM, &handler, NULL);
    


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
    
    
    cleanup(SIGQUIT);
    return 0;
}

