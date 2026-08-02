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

#define PAYLOAD_NAME "dmenu_payload.bin"
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




extern int sceKernelSendNotificationRequest(int, notify_request_t *, size_t, int);
extern int sceKernelGetAppInfo(pid_t pid, app_info_t *info);
// extern int sceKernelLoadStartModule(const char *name, uint64_t argc, const void *argv, uint32_t flags, void *, int *result);
// extern uint sceShellCoreUtilGetAppLaunchedParamString(int appid, const char * name, char * dst , size_t max);

// Not being used atm.
//! TODO: Reimplement once sigterm is being handled and we can actually close the file handles properly
static int logfile;

static const ndgame_t * nd_game_ptr;

static int watchpoint_count;













int echo(const char * str, ...)
{
    int len;
    va_list list;
    va_start(list, str);
    
    char msg[4000];
    len = vsnprintf(msg, sizeof(msg) - 2, str, list);
    
    if (nd_game_ptr)
    {
        klog_printf("%d:%d: %s\n", getpid(), nd_game_ptr->pid, msg);
    }
    else{
        klog_printf("%d:%d: %s\n", getpid(), 0, msg);
    }

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


const ndgame_t * get_nd_game()
{
    return nd_game_ptr;
}

int * get_watchpoint_count()
{
    return &watchpoint_count;
}















// Wait for an eboot.bin process to be found, then connect and attach to it if it's title id matches one of the known ones
// Saves the pid, app id, and title id of the attached process to the provided nd_game struct
// 
//! TODO: Add support for the other naughtydog executable names so the payload works regardless of which is running
static int wait_for_game_launch(ndgame_t * nd_game)
{
    pid_t pid, skipped_ids[0x10];

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


    // Check for any running eboot.bin //! remember to check for the other possible T2R executable names
_retry:
    pid = find_pid("eboot.bin", null);

    if (pid <= 0)
    {
        echo("Unable to find eboot, waiting a few seconds before retrying");
        
        do {
            sleep(7);
            pid = find_pid("eboot.bin", null);
        }
        while (pid <= 0);

        // process started- wait 25 seconds for the game to finish launching
        sleep(25);
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
            echo("Saving app info...");
            nd_game->pid = pid;
            nd_game->app_id = app_info.app_id;
            strncpy(nd_game->title_id, app_info.title_id, 0xA);
            
            int ret;
            if ((ret = attach(*nd_game)))
            {
                return 1;
            }


            notify("\nGame Info: \n  -> Title ID: %s\n  -> App ID: %lX\n  -> Process ID: %d", nd_game->title_id, nd_game->app_id, pid);
            
            proc_wait(nd_game, 0);
            return 0;
        }
    }


    echo("Application did not match any known title ids, adding pid %d to list of ingored processes, and retrying in 5 seconds", pid);
    skipped_ids[attempts++] = pid; // Only add the pid to skipped processes if we don't recognize the title id
    sleep(5);
    goto _retry;
}






static int find_and_trace_game(ndgame_t * nd_game)
{
    // Reset the current game instance
    bzero(nd_game, sizeof(ndgame_t));

    // unsigned long id_check;
    unsigned char retries = 0;
    while (wait_for_game_launch(nd_game) && (retries < (unsigned char) 4))
    {
        ++retries;
        echo("Error occured when waiting for game launch, waiting moment and retrying (5 seconds)");
        sleep(5);
        echo("Retrying now.");
    }

    

    
    // switch((id_check = *(unsigned long *) ((unsigned long) nd_game.title_id) + 1))

    switch(* ((unsigned long *) ((unsigned long) nd_game->title_id + 1)))
    {
        // T2R
        case 3618138931251467088: // PSA10662
        case 4048794567581979472: // PSA15508
        case 3544672884293194576: // PSA15511
        case 3616730478331122512: // PSA15512
            nd_game->game_id = T2R;
            
            nd_game->dev_menu_base = T2RDMENU_DEV_MENU_BASE_PTR;
            nd_game->quick_menu_base = T2RDMENU_QUICK_MENU_BASE_PTR;
            nd_game->favorites_menu_base = T2RDMENU_FAVORITES_MENU_BASE_PTR;

            nd_game->dat_novis = T2R_NOVIS;
            nd_game->func_malloc = T2R_MALLOC;
            nd_game->func_free = T2R_FREE;
            nd_game->func_lookup_symbol = T2R_LOOKUPSYMBOL;
            break;

        // T1X
        case 3907210446666552144: // PSA03396
        case 3617576011346039632: // PSA07642 
        case 3689633605383967568: // PSA07643
            nd_game->game_id = T1x;
            
            nd_game->dev_menu_base = T1xDMENU_DEV_MENU_BASE_PTR;
            nd_game->quick_menu_base = T1xDMENU_QUICK_MENU_BASE_PTR;
            nd_game->favorites_menu_base = T1xDMENU_FAVORITES_MENU_BASE_PTR;

            nd_game->dat_novis = T1x_NOVIS;
            nd_game->func_malloc = T1x_MALLOC;
            nd_game->func_free = T1x_FREE;
            nd_game->func_lookup_symbol = T1x_LOOKUPSYMBOL;
            break;


        default:
            echo("Unknown game title id read (%llu)", *((unsigned long *) ((unsigned long) nd_game->title_id + 1)));
            detach(nd_game, __FUNCTION__);
            bzero(nd_game, sizeof(ndgame_t));
            return 1;
    }

    echo("Game id: %d", nd_game->game_id);
    return 0;
}








void print_exit_message()
{
    klog_printf("\n========================\n| DMenu Payload Exited |\n========================\n\n");
}



void cleanup(int exit_code)
{
    echo("Running cleanup");

    if (nd_game_ptr && nd_game_ptr->pid)
    {
        if (proc_stop(nd_game_ptr->pid))
        {
            echo("Unable to suspend process %d", nd_game_ptr->pid);
        }
        proc_wait((ndgame_t *) nd_game_ptr, false);
        
        echo("Clearing watchpoints and detaching from process %d", nd_game_ptr->pid);
        clear_watchpoint(nd_game_ptr->pid, -1);

        detach((ndgame_t *) nd_game_ptr, __FUNCTION__);
    }

    if (logfile)
    {
        echo("Closing logfile handle");
        close(logfile);
    }

    echo("Exiting");
    exit(exit_code);
}



void sighandler(int signal)
{
    int exit_code = signal == SIGSEGV;

    if (exit_code)
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

    cleanup(exit_code);
}








// Main payload loop (well, it will be a loop eventually)
static int start(ndgame_t * nd_game, int module_pid)
{
    int ret;

_start:
    ret = find_and_trace_game(nd_game);
    if (ret)
    {
        echo("ERROR: Unable to load game");
        sleep(5);
        goto _start;
    }


    // Make sure novis is present
    // add_novis_option(*nd_game);

    if (1)
    {
        goto _ffs;
        dmenu_item_t item;
        dmenu_item_t * mancam_ptr;
        int wp_index;
        
        mancam_ptr = find_item(*nd_game, "Manual Camera", 1);
        
        if (!mancam_ptr)
        {
            echo("Unable to find manual camera item.");
            goto _skip_wp;
        }

        load_item(nd_game->pid, mancam_ptr, &item);
        
        // Address isn't gonna change any time soon, so we'll just hardcode it and skip getting it.
        _ffs:
        wp_index = * get_watchpoint_count();
        
        ret = set_watchpoint(nd_game->pid, wp_index, (void *) 0x12B8F80, DBREG_DR7_EXEC, DBREG_DR7_LEN_1);
        if (ret)
        {
            echo("Error setting watchpoint, aborting as something has gone wrong.");
            return 1;
        }

        // Resume the process and wait for the watchpoint to be hit
        proc_continue(nd_game->pid);
        echo("waiting for watchpoint to be hit");

        pid_t stopped_thread;
        stopped_thread = proc_wait(nd_game, true);
        if (stopped_thread == -1)
        {
            echo("Error awaiting for mancam eh function watchpoint to be hit");
            return 1;
        }

        echo("==================");
        echo("| Watchpoint Hit |");
        echo("==================");
        
        clear_watchpoint(nd_game->pid, wp_index);
    }
    _skip_wp:{}

    
    return 0;

    // Get args to modify and pass as args //! this is clunky
    struct reg args;
    if ((ret = proc_get_regs(nd_game->pid, &args)) != 0)
    {
        echo("Unable to get register values, aborting 'cause something's clearly wrong.");
        return ret;
    }


    args.r_rsi = 5;
    unsigned long long check; 
    if ((ret = proc_call(nd_game, (unsigned long *) 0x16cc530, (unsigned long *) &check, &args)) != 0)
    {
        echo("ERROR: %s call failed! (ret: %d)", __FUNCTION__);
    }
    echo("waiting for function to return");

    // proc_continue(nd_game->pid);

    return 0;
}




int main()
{
    // Set thread name
    syscall(SYS_thr_set_name, -1, PAYLOAD_NAME);
    
    // Set some handlers for secleanup shit so I can stop rebooting the game every time the payload is killed
    struct sigaction handler;
    bzero(&handler, sizeof(struct sigaction));
    handler.__sigaction_u.__sa_handler = sighandler;
    
    atexit(print_exit_message);

    sigaction(SIGTERM, &handler, NULL);
    sigaction(SIGSEGV, &handler, NULL);





    // Kill any previous instances
    pid_t pid;
    if ((pid = find_pid(PAYLOAD_NAME, NULL)) > 0)
    {
        // int saved_pids = 0;
        // pid_t pids_to_skip[0x10];
        
        do {
            printf("Killing old instance (pid: %d)...\n", pid);

            if (kill(pid, SIGTERM))
            {
                echo("SIGTERM ERROR (%d)", pid);
                break;
            }

            
            notify("Sent termination signal to previous %s instance", PAYLOAD_NAME);
            // pids_to_skip[saved_pids++] = pid;
        }
        while (0); //((pid = find_pid(PAYLOAD_NAME, pids_to_skip)) > 0);


        // echo("exiting");
        // exit(0);

        // return 1;
    }

    
    // notify("Test Payload Message");
    // return 0;




    //#
    //## Payload start
    //#

    // Assign values to global variables
    pid_t this;
    this = getpid();
    echo("Proceeding with new %s instance (%d)", PAYLOAD_NAME, this);

    static ndgame_t nd_game;
    nd_game_ptr = (const ndgame_t *) &nd_game;

    watchpoint_count = 0;
    

    // Paylod start
    start(&nd_game, this);
    
    cleanup(0);
    return 0;
}

