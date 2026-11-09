/*
Credits so far:

- John Tornblom (find_pid function)
- Bestpig (va_list/args & kqueue/kevent stuff for monitoring game launch/exit were both learned from backpork. Read the docs so I could write and understand both myself, though)
- Illusiony (told about sceShellCoreUtilGetAppLaunchedParamString)
- earthonion (told me how to link with other libraries)
- AlAzif (can't think of anything at the moment, but very likely)
- LM (force kill process method that works better than sigkill)
*/


#include "inc/include.h"
#include "inc/proc.h"
#include "inc/memory.h"
#include "inc/dmenu.h"
#include "inc/support_tables.h"


//# external functions to link
extern int sceKernelSendNotificationRequest(int, notify_request_t *, size_t, int);
extern int sceKernelGetAppInfo(pid_t pid, app_info_t *info);
// extern int sceKernelLoadStartModule(const char *name, uint64_t argc, const void *argv, uint32_t flags, void *, int *result);
// extern int sceShellCoreUtilGetAppLaunchedParamInt(const uint32_t appid, const char * key, int* out);
extern int sceShellCoreUtilGetAppLaunchedParamString(const uint32_t appid, const char * key, char* out, size_t out_sz);
// extern int sceKernelTerminateProcess(pid_t, int *);





//#
//## Variable Declarations
//#

// File handle for open logfile being saved in console's /data folder
static int logfile;

static NdGame_t ndgame;
NdGame_t * game;

static int watchpoint_count = 0;


















//#
//## Common Function Declarations
//#

void failed_echo(const char * string, ...)
{
#ifdef ECHO
    klog_printf("Segfault received on attempt to print string from address 0x%llX", (unsigned long) string);
#endif
}

// Print (and optionally format) the specified string to kernel output, as well as to the log file if one is active
int echo(const char * string, ...)
{
#ifdef ECHO
    // struct sigaction handler;
    // bzero(&handler, sizeof(struct sigaction));

    // handler.__sigaction_u.__sa_handler = failed_echo;
    // sigaction(SIGSEGV, &handler, NULL);

    int len;
    va_list list;
    va_start(list, string);
    
    char msg[4000];
    len = vsnprintf(msg, sizeof(msg) - 2, string, list);
    
    if (game != NULL)
    {
        klog_printf("%d:%d: %s\n", getpid(), game->pid, &msg);
    }
    else{
        klog_printf("%d:%d: %s\n", getpid(), 0, &msg);
    }

    log_message("%s\n", msg);
    va_end(list);

    

    if (len >= 0x1FA)
    {
        klog_printf("\nWARNING: constructed output string for proceeding message is too long (0x%x >= 0x%x)\n", len, 0x1FA);
    }

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
#ifdef LOGFILE
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
            klog_printf("Unable to open logfile, maybe error code: %s", strerror(errno));
        }
    }


    if (logfile)
    {
        // Write the message to the log file
        int ret;
        ret = write(logfile, message, strlen(message));
        if (ret == -1)
        {
            klog_printf("Write returned error? (%s)\n", strerror(errno));
        }
    }
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
    vsnprintf(message, sizeof(message) - 1, format, list);
    va_end(list);
    return message;
#else
    static char * msg = " ";
    return msg;
#endif
}








int decode_sid(SID SID, const char * DecodedSID)
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

    for (unsigned i = 0; i < input_len; ++i)
    {
        hash ^= input[i];
        hash *= prime;
    }

    return hash;
}









// Get the length of the longest line in a string with line breaks/newlines
// Returns the length on success, or -1 if the pointer is null
size_t get_longest_line_length(const char * str, size_t str_maxlen)
{
    unsigned biggest = 0,
        current = 0;


    if (str != null && str_maxlen > 0)
    {
        for(unsigned i = 0; i < str_maxlen; i++)
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








// Force kill process the proper way, from LM
void ForceKillProc(pid_t pid)
{
    if (pid < 0)
    {
        echo("Invalid PID: %d\n", pid);
        return;
    }

    int ret;
    pid_t this;
    uintptr_t authid;
    #define DECID_AUTH_ID 0x4800000000000022 // required for killing with sceKernelTerminateProcess / sys_proc_term  syscall


    this = getpid();
    authid = kernel_get_ucred_authid(this);
    kernel_set_ucred_authid(this, DECID_AUTH_ID);


    ret = syscall(0x28C, pid);
    if (ret != 0)
    {
        echo("Failed to terminate process with PID: %d. ret: %d, error: %s\n", pid, ret, strerror(errno));
    }
    else {
        echo("Successfully terminated pid: %d\n", pid);
    }

    #undef DECID_AUTH_ID
    kernel_set_ucred_authid(this, authid); // Restore original authid
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
    __rerrv(ret, 1, "Error getting app info, aborting")
    
    game->pid = pid;
    strncpy(game->title_id, app_info->title_id, sizeof(app_info->title_id));

    ret = sceShellCoreUtilGetAppLaunchedParamString(app_info->app_id, "TITLE", game->title, sizeof(game->title));
    __rerrv(ret != 0, 1, "TITLE Err")

    ret = sceShellCoreUtilGetAppLaunchedParamString(app_info->app_id, "CONTENT_VERSION", game->app_ver, sizeof(game->app_ver));
    __rerrv(ret != 0, 1, "CONTENT_VERSION Err")
    
    if (!*(char *) game->app_ver) // Read legacy app parameter name (for PS4 applications)
    {
        ret = sceShellCoreUtilGetAppLaunchedParamString(app_info->app_id, "APP_VER", game->app_ver, sizeof(game->app_ver));
        __rerrv(ret != 0,  1, "APP_VER Err")
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
        echo("_find_pid: %d", pid);

        if (pid > 0) {
            echo("Game launched (%d), waiting an extra 15 seconds for the game to fully launch", pid); // Also to wait for klog output to calm down
            sleep(15);
        }
    }

    return pid;
}








static game_id_t find_and_trace_game()
{
    pid_t pid;
    game_id_t err;
    bzero(game, sizeof(NdGame_t));


    // Wait for a new bigapp process launch
    err = (game_id_t) -1;
    pid = wait_for_game_launch();

    __rerrv(pid < 1, err, "ERROR: Invalid process id returned from launched game (%d), aborting until new app launch", pid);


    // Read and check Title ID
    int ret;
    app_info_t app_info;
    ret = sceKernelGetAppInfo(pid, &app_info);
    __rerrv(ret != 0, err, "ERROR: sceKernelGetAppInfo Failed!!! Aborting payload.");
    
    game_id_t game_id;
    game_id = get_game_id(encode_sid(app_info.title_id));

    if (game_id == NONE)
    {
        echo("Unknown game, ignoring %s.", app_info.title_id);
        return NONE;
    }


    // Attach to new game process
    ret = attach(pid);
    __rerrv(ret, err, "ERROR: Unable to attach to process, aborting.");

    // Wait for the process to stop
    ret = proc_wait(true);
    __rerrv(ret == -1, err, "ERROR: proc_wait returned error when waiting after initial attachment, aborting")


    // Save id of process only once attached
    game->game_id = game_id;

    // Read application metadata (app version and title)
    ret = load_ndgame(game_id, pid, &app_info);
    __rerrv(ret, err, "ERROR: Failed loading ndgame, aborting.");




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
    

    if (game != 0 && game->pid)
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
    echo("game (0x%llX) || game->pid == 0");


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




//! TODO: Give this the same output formatting as the error macro's output
void AssertionHandler(int signal, siginfo_t * info, void * vcontext)
{
    int exitCode;
    exitCode = signal == SIGSEGV;
    
    ucontext_t * context;
    context = (ucontext_t *) vcontext;

    #ifdef VERBOSEDEBUG
    echo("pre-assertion context:\n# mc_addr: %llX\n# mc_rip: %llX\n# mc_flags: %llX", context->uc_mcontext.mc_addr, context->uc_mcontext.mc_rip, context->uc_mcontext.mc_flags);
    #endif

    if (exitCode)
    {
        char assertionMessage[0x200], registersMessage[0x200], bar[0x50];
        
        const char * assertionFormatString = "# Segmentation fault received.\n# \n# Reason:\n#";

        if (info->si_code == SEGV_MAPERR)
        {
            if (info->si_addr == nullptr)
            {
                snprintf(assertionMessage, sizeof(assertionMessage), "%s Null pointer", assertionFormatString);
            }
            else{
                snprintf(assertionMessage, sizeof(assertionMessage), "%s Address 0x%llX was invalid", assertionFormatString, (unsigned long long) info->si_addr);
            }
        }
        else if (info->si_code == SEGV_ACCERR)
        {
            snprintf(assertionMessage, sizeof(assertionMessage), "%s Access to 0x%llX is not permitted", assertionFormatString, (unsigned long long) info->si_addr);
        }
        else{
            snprintf(assertionMessage, sizeof(assertionMessage), "%s Unknown Cause %d; Cause address: 0x%llX", assertionFormatString, info->si_code, (unsigned long long) info->si_addr);
        }

        

        if (0) // Disabled for now, seems off (like RIP always being 1)
        snprintf(registersMessage, sizeof(registersMessage),
            "# Registers:\n"
            "# rax: %lX\n# rbx: %lX\n"
            "# rcx: %lX\n# rdx: %lX\n"
            "# rsi: %lX\n# rdi: %lX\n"
            "# rbp: %lX\n# rsp: %lX\n"
            "# r8:  %lX\n# r9:  %lX\n"
            "# r10: %lX\n# r11: %lX\n"
            "# r12: %lX\n# r13: %lX\n"
            "# r14: %lX\n# r15: %lX\n"
            "# rip: %lX\n# \n# eflags: %ld\n"
            "# trapno:  %d\n# err: %ld",

            context->uc_mcontext.mc_rax,    context->uc_mcontext.mc_rbx,
            context->uc_mcontext.mc_rcx,    context->uc_mcontext.mc_rdx,
            context->uc_mcontext.mc_rsi,    context->uc_mcontext.mc_rdi,
            context->uc_mcontext.mc_rbp,    context->uc_mcontext.mc_rsp,
            context->uc_mcontext.mc_r8,     context->uc_mcontext.mc_r9,
            context->uc_mcontext.mc_r10,    context->uc_mcontext.mc_r11,
            context->uc_mcontext.mc_r12,    context->uc_mcontext.mc_r13,
            context->uc_mcontext.mc_r14,    context->uc_mcontext.mc_r15,
            context->uc_mcontext.mc_rip,    context->uc_mcontext.mc_rflags,
            context->uc_mcontext.mc_trapno, context->uc_mcontext.mc_err
        );

        

        // Build enclosing '#' bars
        size_t len;
        len = get_longest_line_length(assertionMessage, sizeof(assertionMessage));
        if (len > 0x50) {
            len = 0x50;
        }
        for (unsigned i = 0; i < sizeof(bar) && i < len + 2; *(bar + i ++) = '#');
        *(bar + len + 2) = 0;

        echo("\n%s\n%s\n# \n%s\n", bar, assertionMessage, bar);
    }
    else if (signal == SIGTERM)
    {
        echo("Process termiation signal received, process exiting without error.");
    }
    else {
        echo("Miscellaneous signal recieved, exiting (sig: %d)", signal);
    }

    cleanup(exitCode);
}




#ifdef TESTS
//### Testing shit
static void debug_misc()
{
    dump_menu_entries();
return;

    struct reg args;
    uintptr_t buf;

    // Allocate arg data space in game process
    buf = (uintptr_t) proc_malloc(0x200);
    echo("Buffer allocated to 0x%llX", buf);

    // Write args
    write_uint(game->pid, (unsigned *) buf, 0x5D);
    write_string(game->pid, (void *) buf + 0x10, __FILE__);
    write_string(game->pid, (void *) buf + strnlen(__FILE__, 0x200 - 0x40), __FUNCTION__);


    // Push stack allocator
    proc_set_args(&args, buf, buf + 0x10, (uintptr_t) __LINE__, buf + 0x50, 0, 0, 0);
    proc_call((void *) 0x4fa710, nullptr, &args);

    // Create Small SpawnAvActor menu
    proc_set_args(&args, 0, (uintptr_t) read_ulong(game->pid, (unsigned long *) &game->Addrs.favorites_menu_group->root_menu), 0, 0, 0, 0, 0);
    proc_call((void *) 0x27ad490, nullptr, &args);

    // Pop stack allocator
    proc_clear_args(&args);
    proc_call((void *) 0x5eebd0, nullptr, &args);

}
#endif








// Main payload loop (well, it will be a loop eventually)
static int start()
{    
    int ret;
    // Make sure novis is present
    ret = patch_game();

    return ret == 1;
}








int main()
{
    //#
    //## Payload INIT
    //#
    
    // Set process name
    syscall(SYS_thr_set_name, -1, PAYLOAD_NAME);

    echo("\n"
        "====================================\n"
        "| %s Entry.\n"
        "| Built: %s W/ C %ld\n"
        "| From: %s\n"
        "====================================\n",
        
        PAYLOAD_NAME,
        __DATE__, __STDC_VERSION__,
        __FILE__
    );


    // Set handler to print exit message
    atexit(print_exit_message);
    
    
    // Kill any previous instances
    pid_t pid;
    const int waitTime = 4;
    if ((pid = find_pid(PAYLOAD_NAME)) > 0)
    {
        echo("Terminating old %s instance (pid: %d)...\n", PAYLOAD_NAME, pid);
        
        kill(pid, SIGTERM);

        echo("\n# Sent termination signal to previous %s instance %d\n# Waiting %d seconds before force-killing process if it's still active", PAYLOAD_NAME, pid, waitTime);
        sleep(waitTime);

        if (find_pid(PAYLOAD_NAME) == pid)
        {
            echo("\n# Process %d still running %d seconds after SIGTERM; force-killing process.", pid, waitTime);
            ForceKillProc(pid);
            sleep(waitTime);

            if (find_pid(PAYLOAD_NAME) == pid)
            {
                echo("\n# WARNING: Previous %s instance(s) still running (%d)\n", PAYLOAD_NAME, pid);
            }
        }
        else {
            echo("Process %d terminated without issue", pid);
        }


        exit(0);
        return 0;
    }



    // Set some handlers for cleanup shit so I can stop rebooting the game every time the payload is killed
    struct sigaction handler;
    bzero(&handler, sizeof(struct sigaction));

    handler.__sigaction_u.__sa_sigaction = AssertionHandler;
    handler.sa_flags = SA_SIGINFO;
    sigaction(SIGTERM, &handler, NULL);
    sigaction(SIGSEGV, &handler, NULL);

    int ret;
    game = &ndgame;
    watchpoint_count = 0;
    
    
    
    // Find game process to trace
    echo("Proceeding with new %s instance (%d) - Waiting/Searching for game process.", PAYLOAD_NAME, getpid());
    ret = find_and_trace_game();
    __rerrv(ret != 0, 1, "Unhandled error finding game process, payload aborting with error.");



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
    ret = start();
    __err(ret, "Payload start exited with error");

    // Payload exit cleanup
    cleanup(ret);
    return ret;
}

