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

#include "inc/proc.h"
#include "inc/memory.h"
#include "inc/dmenu.h"



//# external functions to link
extern int sceKernelSendNotificationRequest(int, notify_request_t *, size_t, int);
extern int sceKernelGetAppInfo(pid_t pid, app_info_t *info);
// extern int sceKernelLoadStartModule(const char *name, uint64_t argc, const void *argv, uint32_t flags, void *, int *result);
// extern uint sceShellCoreUtilGetAppLaunchedParamString(int appid, const char * name, char * dst , size_t max);







//#
//## Variable Declarations
//#

// File handle for open logfile being saved in console's /data folder
static int logfile;
static database_t * db;

static ndgame_t * nd_game_ptr;

static int watchpoint_count;















//#
//## Common Function Declarations
//#

// Print (and optionally format) the specified string to kernel output, as well as to the log file if one is active
int echo(const char * string, ...)
{
#ifdef DEBUG
    int len;
    va_list list;
    va_start(list, string);
    
    char msg[4000];
    len = vsnprintf(msg, sizeof(msg) - 2, string, list);
    
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
#else
    return 0;
#endif
}








// Send a notification request to the ps4 with the provided (optionally formatted) message
void notify(const char * message, ...)
{
    va_list list;
    notify_request_t request = { };

    // Build string with variable arguments
    va_start(list, message);
    vsnprintf(request.message, sizeof(request.message) - 1, message, list);
    va_end(list);
    
#ifdef NOTIFY
    sceKernelSendNotificationRequest(0, &request, sizeof(request), 0); // Not sure what the first and last arg are for. One may be notification type though
#endif
#ifdef DEBUG
    printf("[Notification]: %s\n", request.message);
#endif
}








// Log (and optionally format) a message to an active log file on the ps5 at /data/bLog.txt
void log_message(const char * base_string, ...)
{
#ifdef LOGOUTPUT
#ifdef DEBUG
    errno = 0;
    va_list args;
    char message[4000];

    va_start(args, base_string);
    vsnprintf(message, sizeof(message), base_string, args);
    va_end(args);



    if (logfile == 0)
    {
        logfile = open("/data/bLog.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

        if (logfile < 0)
        {
            printf("Unable to open logfile, maybe error code: %s", strerror(errno));
        }
    }


    if (logfile)
    {
        // Write the message to the log file
        write(logfile, message, strlen(message));
    }
#endif
#endif
}








// Construct one string in memory to be returned C#-style. Uses one static buffer, only use for certain prints (like the assertion macro)
char * strbufbuild(char * format, ...)
{
    va_list list;
    static char message[2000];
    bzero(message, sizeof(message));

    // Build string with variable arguments
    va_start(list, format);
    vsnprintf(message, sizeof(message) - 1, message, list);
    va_end(list);
    
    return message;
}








// Get the length of the longest line in a string with line breaks/newlines
// Returns the length on success, or -1 if the pointer is null
int get_longest_line_length(const char * str, size_t str_maxlen)
{
    int biggest = 0,
        current = 0;


    
    if (str != null && str_maxlen > 0)
    {
        for(int i = 0; i < str_maxlen; i++)
        {
            if (*(str + i) == '\n')
            {
                if (current > biggest)
                {
                    biggest = current;
                }
                current = 0;
                continue;
            }
            if (*(str + i) == 0)
            {
                goto _ret;
            }

            ++current;
        }

        echo("WARNING: %s: string max length reached before null terminator hit, take longest line result with a grain of salt.", __FUNCTION__);
    }


_ret:
    if (current > biggest)
    {
        biggest = current;
    }
    return biggest;
}








ndgame_t * get_nd_game()
{
    return nd_game_ptr;
}








int * get_watchpoint_count()
{
    return &watchpoint_count;
}








// Code copied from a sample written by john-tornblom
pid_t find_pid(const char *name)
{
    int mib[4] = {1, 14, 8, 0};
    pid_t mypid = getpid();
    pid_t pid = -1;
    size_t buf_size;
    uint8_t *buf;

    if (sysctl(mib, 4, 0, &buf_size, 0, 0)) {
        return -1;
    }

    if (!(buf = malloc(buf_size))) {
        return -1;
    }

    if (sysctl(mib, 4, buf, &buf_size, 0, 0)) {
        free(buf);
        return -1;
    }

    for (uint8_t *ptr = buf; ptr < (buf + buf_size);) {
        int ki_structsize = *(int *)ptr;
        pid_t ki_pid = *(pid_t *)&ptr[72];
        char *ki_tdname = (char *)&ptr[447];

        ptr += ki_structsize;
        if (!strcmp(name, ki_tdname) && ki_pid != mypid)
        {
            pid = ki_pid;
        }
    }

    free(buf);

    return pid;
}








//#
//## Common Function Declarations
//#

// Wait for an eboot.bin process to be found, then connect and attach to it if it's title id matches one of the known ones
// Saves the pid, app id, and title id of the attached process to the provided nd_game struct
// 
//! TODO: Add support for the other naughtydog executable names so the payload works regardless of which is running
static int wait_for_game_launch(ndgame_t * nd_game)
{
    pid_t pid, skipped_ids[0x10];

    int ret, attempts = 0;
    char * titleids[] =
    {
        // R2R Ids
        "PPSA10662",
        "PPSA15508",
        "PPSA15511",
        "PPSA15512",

        // T1X Ids
        "PPSA03396",
        "PPSA07642",
        "PPSA07643"
    };


    // Check for any running eboot.bin //! remember to check for the other possible T2R executable names
_retry:
    pid = find_pid("eboot.bin");

    if (pid <= 0)
    {
        echo("Unable to find eboot, waiting a few seconds before retrying");
        
        do {
            sleep(7);
            pid = find_pid("eboot.bin");
        }
        while (pid <= 0);
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
    


    
    echo("Found eboot.bin (pid %d), reading app info...", pid);
    app_info_t app_info;
    ret = sceKernelGetAppInfo(pid, &app_info);
    if (ret)
    {
        echo("Error getting app info, retrying in 5 seconds");
        sleep(5);
        goto _retry;
    }
    



    echo("Read app info, checking for known title id");
    for (int i = 0; i < sizeof(titleids) / 8; i++)
    {
        if (!strncmp(app_info.title_id, titleids[i], 0xA))
        {
            echo("Known title id found, saving app info");
            nd_game->pid = pid;
            nd_game->app_id = app_info.app_id;
            strncpy(nd_game->title_id, app_info.title_id, 0xA);
            
            int ret;
            if ((ret = attach(nd_game)))
            {
                return 1;
            }


            notify("\nGame Info:\n  -> Title ID: %s\n  -> App ID: %lX\n  -> Process ID: %d\n", nd_game->title_id, nd_game->app_id, pid);
            
            proc_wait(nd_game, true);
            return 0;
        }
    }


    echo("App's title id did not match any known, adding pid %d to list of ingored processes, and retrying in 5 seconds", pid);
    skipped_ids[attempts++] = pid; // Only add the pid to skipped processes if we don't recognize the title id
    sleep(5);
    goto _retry;
}








static int find_and_trace_game(ndgame_t * ndgame)
{
    // Reset the current game instance
    bzero(ndgame, sizeof(ndgame_t));

    // unsigned long id_check;
    if (wait_for_game_launch(ndgame))
    {
        echo("An error occured while waiting for game launch, aborting payload.");
        return 1;
    }

    


    //! TODO: Load and check app.db for the game's app_ver
    switch(* ((unsigned long *) ((unsigned long) ndgame->title_id + 1)))
    {
        // T2R
        case 3618138931251467088: // PSA10662
        case 4048794567581979472: // PSA15508
        case 3544672884293194576: // PSA15511
        case 3616730478331122512: // PSA15512
            ndgame->game_id = T2R;
            
            ndgame->dev_menu_base       = (dmenu_menu_base_t *) read_ulong(ndgame->pid, (unsigned long *) T2RDMENU_DEV_MENU_BASE_PTR);
            ndgame->quick_menu_base     = (dmenu_menu_base_t *) read_ulong(ndgame->pid, (unsigned long *) T2RDMENU_QUICK_MENU_BASE_PTR);
            ndgame->favorites_menu_base = (dmenu_menu_base_t *) read_ulong(ndgame->pid, (unsigned long *) T2RDMENU_FAVORITES_MENU_BASE_PTR);

            ndgame->dat_novis = T2R_NOVIS;
            ndgame->func_malloc = T2R_MALLOC;
            ndgame->func_free = T2R_FREE;
            ndgame->func_lookup_symbol = T2R_LOOKUPSYMBOL;
            break;

        // T1X
        case 3907210446666552144: // PSA03396
        case 3617576011346039632: // PSA07642 
        case 3689633605383967568: // PSA07643
            ndgame->game_id = T1x;
            
            ndgame->dev_menu_base       = (dmenu_menu_base_t *) read_ulong(ndgame->pid, (unsigned long *) T1XDMENU_DEV_MENU_BASE_PTR);
            ndgame->quick_menu_base     = (dmenu_menu_base_t *) read_ulong(ndgame->pid, (unsigned long *) T1XDMENU_QUICK_MENU_BASE_PTR);
            ndgame->favorites_menu_base = (dmenu_menu_base_t *) read_ulong(ndgame->pid, (unsigned long *) T1XDMENU_FAVORITES_MENU_BASE_PTR);

            ndgame->dat_novis = T1X_NOVIS;
            ndgame->func_malloc = T1X_MALLOC;
            ndgame->func_free = T1X_FREE;
            ndgame->func_lookup_symbol = T1X_LOOKUPSYMBOL;
            break;


        default:
            echo("Unknown game title id read (%llu)", *((unsigned long *) ((unsigned long) ndgame->title_id + 1)));
            // detach(ndgame, __FUNCTION__);
            // bzero(ndgame, sizeof(ndgame_t));
            return 1;
    }

    echo("Game id: %d", ndgame->game_id);
    return 0;
}








void print_exit_message()
{
    klog_printf("\n=============================\n| DMenu Payload Exited (%d) |\n=============================\n\n", getpid());
}




void cleanup(int exit_code)
{
    echo("\n==================================\n| DMenu Payload Exiting... (%d) |\n==================================", getpid());
    

    if (nd_game_ptr && nd_game_ptr->pid)
    {
        if (proc_stop((ndgame_t *) nd_game_ptr))
        {
            echo("Unable to suspend process %d", nd_game_ptr->pid);
        }
        proc_wait((ndgame_t *) nd_game_ptr, false);
        
        if (get_watchpoint_count())
        {
            echo("Clearing watchpoints and detaching from process %d", nd_game_ptr->pid);
            clear_watchpoint((ndgame_t *) nd_game_ptr, -1);
        }

        detach((ndgame_t *) nd_game_ptr, __FUNCTION__);
    }


    if (logfile) {
        echo("Closing logfile handle");
        close(logfile);
    }
    if (db) {
        echo("Freeing database file pointer (0x%llX)", db);
        free(db);
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
static int start(ndgame_t * ndgame, int module_pid)
{
    int ret;

    // Find game process to trace
    ret = find_and_trace_game(ndgame);
    __rerr(ret, "Unhandled error finding game process, payload aborting.", 1);




    // Make sure novis is present
    ret = add_novis_option(ndgame, menu_index_quick);
    if (ret)
    {
        echo("Error adding novis option, proceeding processes may also fail.\n");
    }

    return ret != 0;
}








int main()
{
    //#
    //## Payload setup/init
    //#

    // Set process name
    syscall(SYS_thr_set_name, -1, PAYLOAD_NAME);

    // Set handler to print exit message (cleanup not needed yet)
    atexit(print_exit_message);



    // Kill any previous instances
    errno = 0;
    pid_t pid;
    if ((pid = find_pid(PAYLOAD_NAME)) > 0)
    {
        do {
            printf("Killing old instance (pid: %d)...\n", pid);
            
            if (kill(pid, SIGTERM))
            {
                echo("ERROR: SIGTERM on process (%d) failed!!! - No idea how this failed, but something's clearly wrong. Off we screw.", pid);
                if (errno)
                {
                    echo("kill(%d, SIGTERM): %s", pid, strerror(errno));
                }
                return -1;
            }


            notify("Sent termination signal to previous %s instance %d", PAYLOAD_NAME, pid);
            sleep(2);
        }
        while ((pid = find_pid(PAYLOAD_NAME)) > 0);

        exit(0);
        return 0;
    }
    


    // Set some handlers for cleanup shit so I can stop rebooting the game every time the payload is killed
    struct sigaction handler;
    bzero(&handler, sizeof(struct sigaction));

    handler.__sigaction_u.__sa_handler = sighandler;
    sigaction(SIGTERM, &handler, NULL);
    sigaction(SIGSEGV, &handler, NULL);



    




    
    //#
    //## Global/local variable assignments/declarations
    //#


    int ret;
    errno = 0;
    pid_t this;
    watchpoint_count = 0;
    static ndgame_t ndgame;
    nd_game_ptr = (ndgame_t *) &ndgame;

    this = getpid();
    __rerr(!this || errno, "ERROR: getpid returned error?? What on earth have you done?! (or something else set errno)", 1);
    echo("Proceeding with new %s instance (%d)", PAYLOAD_NAME, this);
    
    





    
    //#
    //## Testing shit START
    //#
    #ifdef DEBUG
    {
        goto _func_issues_testing;

        //#
        //## app.db loading tests
        //#
        struct stat s;

        const char * db_path;
        db_path = "/system_data/priv/mms/app.db";


        ret = stat(db_path, &s);
        __err(ret != 0, "ERROR: Failed to stat app.db, unable to get app title and version");

        if (ret == 0)
        {
            errno = 0;
            echo("app.db size: 0x%llX", s.st_size);

            __rerr(db != 0, "ERROR: database file pointer was somehow non-null already. check shit.", 1);

            size_t size;
            size = align(s.st_size);
            echo("Allocating %llX bytes", size);
            db = malloc(size);
            __rerr(!db, "ERROR: Unable to allocate the %d bytes requested for loading the database file in to memory", 1);

            int handle;
            handle = open(db_path, O_RDONLY);
            __rerr(!handle, "ERROR: Unable to open the database file for reading.", 1);
            
            ret = read(handle, db, s.st_size);
            __rerr(!ret || ret != s.st_size, "ERROR: Unable to read database file in to memory.", 1);
            
            close(handle);





            if (strncmp("SQLite format 3", (const char *) db, 0x10))
            {
                echo("ERROR: Invalid magic read from file at %s. %s != SQLite format 3", db_path, (char *) db);
            }
            
            __rerr(sizeof(database_t) != 0x64, strbufbuild("ERROR: database struct length was not equal to the expected 100 bytes (100!=%d)", sizeof(database_t)), 1);
            echo("Magic: %s", (char *) db->magic);

            unsigned short page_len;
            page_len = reverse16(db->block_size);
            
            unsigned int fl_pagecount;
            fl_pagecount = reverse32(db->freelist_page_count);
            
            unsigned int db_size;
            db_size = reverse32(db->database_size);

            echo("Block size: (%x) %d", page_len, page_len);
            echo("Change Count: 0x%lld", reverse32(db->file_change_count));
            echo("Freelist page count: 0x%lld", fl_pagecount);
            if (fl_pagecount)
            {
                echo("First freelist page: 0x%lld", reverse32(db->first_freelist_trunk_page_number));
            }



            echo("First page type: %d", *(((char *) db) + 0x64));

            echo("Database size: 0x%llX (%ld bytes)", db_size, db_size * page_len);
            for (int i = 1, o = page_len; i < db_size; i++, o += page_len)
            {
                echo("Page #%d offset: 0x%llX - 0x%llX / Type: %d", i, o, o + page_len - 1, *((char *) db + o));


            }




            //! TODO: These won't be called if an unhandled error occurs. Add them to cleanup
            if (db)
            {
                free(db);
                db = 0;
            }

            cleanup(0);










            //#
            //## Function Call issues
            //#
        _func_issues_testing:{}
            void * addr;
            struct reg _args;
            unsigned long return_check;

            
            // Find game process to trace
            ret = find_and_trace_game(&ndgame);
            __rerr(ret, "Unhandled error finding game process, payload aborting.", 1);

            

            ret = proc_clear_args(&ndgame, &_args);
            __rerr(ret, "Error clearing args, aborting test call", 1);

            echo("call 1 pre");
            addr = (void *) 0x16cc670; // Toggle menu's open state

            ret = proc_call(&ndgame, addr, &return_check, &_args);
            __rerr(ret, strbufbuild("Error calling test function 0x%llX", addr), 1);
            echo("call 1 post");


            
            // Detonate all enemies
            ret = proc_clear_args(&ndgame, &_args);
            __rerr(ret, "Error clearing args, aborting test call", 1);

            echo("call 2 pre");
            addr = (void *) 0x16cc530;

            ret = proc_call(&ndgame, addr, &return_check, &_args);
            __rerr(ret, strbufbuild("Error calling test function 0x%llX", addr), 1);
            echo("call 2 post");

            return ret != 0;

        }



        return 0;
    }
    #endif



    




    //#
    //## Payload START
    //#
    ret = start(&ndgame, this);
    __err(ret, "Payload start exited with error");

    cleanup(ret);
    return ret;
}

