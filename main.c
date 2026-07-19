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




typedef struct notify_request {
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


typedef enum wp_type {
    WP_CODE, // Break on instruction execution only.
    WP_DATARW, // Break on data writes only.
    WP_IOWR, // Break on I/O reads or writes.
    WP_DATARW_NI  // Break on data reads or writes but not instruction fetches.

} wp_type_t;




extern int sceKernelSendNotificationRequest(int, notify_request_t*, size_t, int);
extern int sceKernelGetAppInfo(pid_t pid, app_info_t *info);
extern int sceKernelLoadStartModule(const char *name, uint64_t argc, const void *argv, uint32_t flags, void *, int *result);

// Not being used atm.
//! TODO: Reimplement once sigterm is being handled and we can actually close the file handles properly
static int logfile;


// Lazy global for handling cleanup on sigterm
static pid_t active_pid;


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


    app_info_t app_info;
    sceKernelGetAppInfo(pid, &app_info);

    if (!strncmp(app_info.title_id, title_id, 10))
    {
        int status;
        if ((status = set_dbg_perms()) != 0)
        {
            return -1;
        }

        echo("Successfully set permissions.");
        
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






static int start(int module_pid)
{
    void * mancam_toggle_eh = (void *) 0x27f2db0;
    int ret, pid;
    pid = wait_for_game_launch("PPSA10662");
    
    active_pid = pid; // Save pid in case of termination
    

    ret = set_watchpoint(pid, 0, mancam_toggle_eh, 0);
    if (ret)
    {
        echo("Error setting watchpoint, aborting.");
        goto _detach;
    }

    // Resume the process now that the breakpoint's set on all threads
    proc_continue(pid, nullptr);

    // Wait for it to be hit
    proc_wait(pid);
    echo("Watchpoint has been passed");
    
    dmenu_menu_base_t base;
    ret = load_menu_base(pid, (void *) T2DMENU_QUICK_MENU_BASE_PTR, &base);
    if (ret)
    {
        echo("Error getting quick menu base, aborting.");
        goto _detach;
    }
    echo("Menu Base");

    dmenu_item_t * item_ptr = find_item(pid, &base, "Nani");
    echo("item ptr (%llX)", item_ptr);

    if (item_ptr <= 0)
    {
        echo("Error getting item, aborting.");
        goto _detach;
    }

    dmenu_item_t item;
    load_item(pid, item_ptr, &item);
    write_string(pid, item.item_text, "Nani");


    _detach:
    clear_watchpoint(pid, 0);
    detach(pid, __FUNCTION__);

    return 0;
}


void cleanup(int singal)
{
    return;
    echo("Termination signal recieved, exiting (%d)", singal);

    if (active_pid)
    {
        detach(active_pid, __FUNCTION__);
    }
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

    
    return 0;
}

