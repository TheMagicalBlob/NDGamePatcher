/*
Credits so far:

- John Tornblom (find_pid function)
- Bestpig (va_list/args & kqueue/kevent stuff for monitoring game launch/exit were both learned from backpork. Read the docs so I could write and understand both myself, though)
- Illusiony (told about sceShellCoreUtilGetAppLaunchedParamString)
- earthonion (told me how to link with other libraries)
- AlAzif (can't think of anything at the moment, but very likely)
*/


#include "inc/include.h"
#include "inc/proc.h"
#include "inc/memory.h"
#include "inc/dmenu.h"
#include "inc/addresses.h"


//# external functions to link
extern int sceKernelSendNotificationRequest(int, notify_request_t *, size_t, int);
extern int sceKernelGetAppInfo(pid_t pid, app_info_t *info);
// extern int sceKernelLoadStartModule(const char *name, uint64_t argc, const void *argv, uint32_t flags, void *, int *result);
// extern int sceShellCoreUtilGetAppLaunchedParamInt(const uint32_t appid, const char * key, int* out);
extern int sceShellCoreUtilGetAppLaunchedParamString(const uint32_t appid, const char * key, char* out, size_t out_sz);






//#
//## Variable Declarations
//#

// File handle for open logfile being saved in console's /data folder
static int logfile;

static ndgame_t ndgame;

static int watchpoint_count = 0;

ndgame_t * game;














//#
//## Common Function Declarations
//#

// Print (and optionally format) the specified string to kernel output, as well as to the log file if one is active
int echo(const char * string, ...)
{
#ifdef ECHO
    int len;
    va_list list;
    va_start(list, string);
    
    char msg[4000];
    len = vsnprintf(msg, sizeof(msg) - 2, string, list);
    
    if (game)
    {
        klog_printf("%d:%d: %s\n", getpid(), game->pid, msg);
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
    echo("[Notification]: %s\n", request.message);
#endif
}








// Log (and optionally format) a message to an active log file on the ps5 at /data/bLog.txt
void log_message(const char * base_string, ...)
{
#ifdef ECHO
#ifdef LOGOUTPUT
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








// [DEBUG-ONLY] Construct one string in memory to be returned C#-style. Uses one static buffer, only use for certain prints (like the assertion macro)
char * strbufbuild(char * format, ...)
{
    #ifdef ECHO
        va_list list;
        static char message[2000];
        bzero(message, sizeof(message));

        // Build string with variable arguments
        va_start(list, format);
        vsnprintf(message, sizeof(message) - 1, message, list);
        va_end(list);
        return message;
    #else
        static char * msg = " ";
        return msg;
    #endif
}







int decode_sid (SID sid)
{
    echo("Not Implemented Exception (%s)", __FUNCTION__);
    kill(getpid(), SIGSEGV);
    return 1;
}







// Encode a provided string as a 64-bit fnva-1 hash
unsigned long encode_sid(const char * input)
{
    if (!input)
    {
        echo("ERROR: Null input string pointer provided for SID encoding.");
        return null;
    }
    if (!*input)
    {
        echo("WARNING: Empty string provided for SID encoding.");
        return (SID) 0;
    }


    SID hash = 14695981039346656037ul;
    unsigned long prime = 1099511628211ul;

    size_t input_len;

    input_len = strlen(input);

    for (int i = 0; i < input_len; ++i)
    {
        hash ^= input[i];
        hash *= prime;
    }

    return hash;
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
//## Main Function Declarations
//#

//! TODO: Add remaining title ids
static game_id_t get_game_id(title_id_check_t check)
{
    // Read from the title id's string * + 1 so we can read it as an unsigned long without missing id's (e.x. reading both PSA15511 and PSA15512 as (ulong)PSA1551)
    switch(check)
    {
        // T2R
        case PPSA10662:
        case PPSA15508:
        case PPSA15511:
        case PPSA15512:
            return T2R;


        // T1X
        case PPSA03396:
        case PPSA07642:
        case PPSA07643:
            return T1X;


        default:
            echo("Unknown game title id read (%llu)", check);
    }

    return NONE;
}






static int load_ndgame(game_id_t game_id, pid_t pid, app_info_t * app_info)
{
    //#
    //## Load app metadata
    //#
    echo("Loading NDGame for pid %d", pid);

    int ret;
    ret = sceKernelGetAppInfo(pid, app_info);
    __rerrv(ret, "Error getting app info, aborting", 1);
    
    game->pid = pid;
    strncpy(game->title_id, app_info->title_id, sizeof(app_info->title_id));

    ret = sceShellCoreUtilGetAppLaunchedParamString(app_info->app_id, "TITLE", game->title, sizeof(game->title));
    __rerrv(ret != 0, "TITLE Err", 1);

    ret = sceShellCoreUtilGetAppLaunchedParamString(app_info->app_id, "CONTENT_VERSION", game->app_ver, sizeof(game->app_ver));
    __rerrv(ret != 0, "CONTENT_VERSION Err", 1);
    
    if (!*(char *) game->app_ver) // Read legacy app parameter name (for PS4 applications)
    {
        ret = sceShellCoreUtilGetAppLaunchedParamString(app_info->app_id, "APP_VER", game->app_ver, sizeof(game->app_ver));
        __rerrv(ret != 0, "APP_VER Err", 1);
    }




    //#
    //## Load function and variable addresses
    //#
    do_load_ndgame(game_id, encode_sid(game->app_ver));
    return 0;
}






// Wait for an eboot.bin process to be found, then connect and attach to it if it's title id matches one of the known ones
// Saves the pid, app id, and title id of the attached process to the provided ndgame struct
// 
//! TODO: Add support for the other naughtydog executable names so the payload works regardless of which is running
static pid_t wait_for_game_launch()
{
    pid_t pid;

    //! TODO: Replace with kernel event method. We've already added credits for it, understand and then use it already
    // Check for any running eboot.bin processes
    pid = find_pid("eboot.bin");
    echo("find_pid: %d", pid);

    while (pid == -1) {
        sleep(10);
        pid = find_pid("eboot.bin");
        echo("find_pid: %d", pid);

        if (pid) {
            echo("Game launched, waiting an extra 15 seconds for the game to fully launch"); // Also to wait for klog output to calm down
            sleep(15);
        }
    }

    return pid;
}








static game_id_t find_and_trace_game()
{
    pid_t pid;
    game_id_t err;
    bzero(game, sizeof(ndgame_t));


    // Wait for a new bigapp process launch
    err = (game_id_t) -1;
    pid = wait_for_game_launch();
    __rerrv(pid < 1, strbufbuild("ERROR: Invalid process id returned from launched game (%d), aborting until new app launch", pid), err)


    // Read and check Title ID
    int ret;
    app_info_t app_info;
    ret = sceKernelGetAppInfo(pid, &app_info);
    __rerrv(ret != 0, "ERROR: sceKernelGetAppInfo Failed!!! Aborting payload.", err);
    
    game_id_t game_id;
    game_id = get_game_id(encode_sid(app_info.title_id));

    if (game_id == NONE)
    {
        echo("Unknown game, ignoring %s.", app_info.title_id);
        return NONE;
    }


    // Attach to new game process
    ret = attach(pid);
    __rerrv(ret, "ERROR: Unable to attach to process, aborting.", err);

    // Wait for the process to stop
    ret = proc_wait(true);
    __rerrv(ret == -1, "ERROR: proc_wait returned error when waiting after initial attachment, aborting", err)


    // Save id of process only once attached
    game->game_id = game_id;

    // Read application metadata (app version and title)
    ret = load_ndgame(game_id, pid, &app_info);
    __rerrv(ret, "ERROR: Failed loading ndgame, aborting.", err);




    notify(
        "Attached to game process. (pid %d)\n"
        "Title ID: %s\n"
        "App Version: %s\n"
        "Title: %s\n",
        
        pid,
        game->title_id,
        game->app_ver,
        game->title
    );

    return ret;
}








void print_exit_message()
{
    klog_printf("\n=============================\n| DMenu Payload Exited (%d) |\n=============================\n\n", getpid());
}




void cleanup(int exit_code)
{
    echo("\n==================================\n| DMenu Payload Exiting... (%d) |\n==================================", getpid());
    

    if (game && game->pid)
    {
        if (proc_stop())
        {
            echo("Unable to suspend process %d", game->pid);
        }
        proc_wait(false);
        
        if (get_watchpoint_count())
        {
            echo("Clearing watchpoints and detaching from process %d", game->pid);
            clear_watchpoint(-1);
        }

        detach(__FUNCTION__);
    }


    if (logfile) {
        echo("Closing logfile handle");
        close(logfile);
    }

    echo("Exiting");
    exit(exit_code);
}



void setup_assertion(const char * function, int line)
{

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




#ifdef TESTS
//### Testing shit
static void debug_misc()
{
    //#
    //## Function Call issues
    //#
    int ret;
    void * addr;
    struct reg _args;
    unsigned long return_check;


    ret = proc_clear_args(&_args);
    __rerr(ret, "Error clearing args, aborting test call");

    echo("call 1 pre");
    addr = (void *) 0x16cc670; // Toggle menu's open state

    ret = proc_call(addr, &return_check, &_args);
    __rerr(ret, strbufbuild("Error calling test function 0x%llX", addr));
    echo("call 1 post");

    
    
    // Detonate all enemies
    if (game->game_id != T2R)
    {
        return;
    }
    ret = proc_clear_args(&_args);
    __rerr(ret, "Error clearing args, aborting test call");

    echo("call 2 pre");
    addr = (void *) 0x16cc530;

    ret = proc_call(addr, &return_check, &_args);
    __rerr(ret, strbufbuild("Error calling test function 0x%llX", addr));
    echo("call 2 post");
}
#endif








// Main payload loop (well, it will be a loop eventually)
static int start(int this)
{    
    int ret;
    // Make sure novis is present
    ret = add_novis_option(menu_index_quick);
    __err(ret, "Error adding novis option, proceeding processes may also fail.\n");

    return ret != 0;
}








int main()
{
    //#
    //## Payload INIT
    //#
    
    // Set process name
    syscall(SYS_thr_set_name, -1, PAYLOAD_NAME);
    
    // Set handler to print exit message
    atexit(print_exit_message);
    
    
    // Kill any previous instances
    int pid,
    tries = 0;
    if ((pid = find_pid(PAYLOAD_NAME)) > 0)
    {
        do {
            echo("Killing old %s instance (pid: %d)...\n", PAYLOAD_NAME, pid);
            
            tries ++;
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
        while ((pid = find_pid(PAYLOAD_NAME)) > 0 && tries < 2);

        if (pid)
        {
            echo("WARNING: Previous %s instance(s) still running (%d)", PAYLOAD_NAME, pid);
        }

        exit(0);
        return 0;
    }



    // Set some handlers for cleanup shit so I can stop rebooting the game every time the payload is killed
    struct sigaction handler;
    bzero(&handler, sizeof(struct sigaction));

    handler.__sigaction_u.__sa_handler = sighandler;
    sigaction(SIGTERM, &handler, NULL);
    sigaction(SIGSEGV, &handler, NULL);

    int ret;
    game = &ndgame;
    watchpoint_count = 0;
    
    
    
    // Find game process to trace
    echo("Proceeding with new %s instance (%d) - Waiting/Searching for game process.", PAYLOAD_NAME, getpid());
    ret = find_and_trace_game();
    __rerrv(ret != 0, "Unhandled error finding game process, payload aborting with error.", 1);



#ifdef TESTS
    //#
    //## Payload DEBUG START
    //#
    debug_misc();
    cleanup(ret);
    return 0;
#endif
    

    
    //#
    //## Payload START
    //#
    
    // Payload start
    ret = start(getpid());
    __err(ret, "Payload start exited with error");

    // Payload exit cleanup
    cleanup(ret);
    return ret;
}

