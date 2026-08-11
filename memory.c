#include "inc/include.h"
#include "inc/dmenu.h"

#define printf(s, ...) klog_printf(s, __VA_ARGS__)

#define _SHELLCORE_AUTHID 0x4800000000010003L



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



// from john-tornblom. Modified to skip known pids for testing purposes
pid_t find_pid(const char *name, pid_t ignored_pids[])
{
    int mib[4] = {1, 14, 8, 0};
    pid_t mypid = getpid();
    pid_t pid = -1;
    size_t buf_size;
    uint8_t *buf;
    int pids_len = -1;

    if (ignored_pids)
    {
        pids_len = 0;

        for(;; pids_len++)
        {
            if (!ignored_pids[pids_len])
            {
                break;
            }
        }
    }

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
            for(int i = 0; i < pids_len;)
            {
                if (ki_pid == ignored_pids[i++])
                {
                    continue;
                }
            }

            pid = ki_pid;
        }
    }

    free(buf);

    return pid;
}




//#
//## ptrace debugging functions
//#

// Set the uncred_authid of the current payload to allow for attaching via ptrace
int set_authid_for_ptrace()
{
    int this = getpid();
    long authid = kernel_get_ucred_authid(this);

    if (authid == _SHELLCORE_AUTHID)
    {
        echo("Auth ID already set");
        return 0;
    }

    return kernel_set_ucred_authid(this, _SHELLCORE_AUTHID);
}






int attach(ndgame_t ndgame)
{
    int ret;
    errno = 0;

    
    ret = set_authid_for_ptrace();
    if (errno)
    {
        echo("ERROR: Unable to set ucred authid for ptrace (Error: %s)", strerror(errno));
        return 1;
    }


    if ((ret = ptrace(PT_ATTACH, ndgame.pid, 0, 0)) != 0)
    {
        notify("PT_ATTACH ERROR: %s\n", strerror(ret));
        
        if(kill(ndgame.pid, SIGCONT))
        {
            echo("WARNING: Unable to resume process through SIGCONT");
        }
    }
    
    echo("Attached to pid %d", ndgame.pid);
    return ret;
}






pid_t proc_wait(ndgame_t * game, int wait);
// //!
int detach(ndgame_t * ndgame, const char * from)
{
    if (ndgame->pid < 1)
    {
        echo("%s: Invalid pid provided (%d).", __FUNCTION__, ndgame->pid);
        return 1;
    }
    echo("Detaching from process %d (%s)", ndgame->pid, from);


    kill(ndgame->pid, SIGSTOP);
    
    int ret;
    ret = proc_wait(ndgame, false);
    if (ret < 0)
    {
        echo("Error waiting for process, but we're exiting anyway");
    }

    errno = 0;
    if ((ret = ptrace(PT_DETACH, ndgame->pid, 0, 0)) != 0)
    {
        if (errno)
        {
            perror("PT_DETACH: ");
        }
        else {
            echo("PT_DETACH: %s (verify code)", strerror(ret));
        }
    }
    else{
        notify("Detached from %d, exiting.", ndgame->pid);
    }

    kill(ndgame->pid, SIGCONT);
    bzero(ndgame, sizeof(ndgame_t));
    return ret;
}






// Wait for the secified process to either be stopped, or exit
// If waitpid experiences an error, returns -1
// otherwise, returns the result of WIFEXITED
pid_t proc_wait(ndgame_t * game, int wait)
{
    int options;
    options = WTRAPPED | WEXITED | WUNTRACED;
    
    if (!wait)
    {
        options |= WNOHANG;
        echo("waiting for pid %d w/ NOHANG", game->pid);
    }
    else{
        echo("waiting for pid %d.", game->pid);
    }
    pid_t pid;


    pid = waitpid(game->pid, &game->status, options); // WTRAPPED and WEXITED are set implicitly for waitpid, but meh
    if (pid == -1)
    {
        if (errno == ECHILD)
        {
            echo("waitpid: no children not previously awaited (?\?)");
        }
        else {
            echo("waitpid failed: %s", strerror(errno));
        }
        
        return -1;
    }
    echo("Process stopped with status: 0x%llX.", game->status);
    
    if (pid)
    {
        //! TODO: Finish writing this.
        int ret;
        errno = 0;

        struct ptrace_lwpinfo thread_info;
        ret = ptrace(PT_LWPINFO, pid, (caddr_t) &thread_info, sizeof(struct ptrace_lwpinfo));
        if (ret != 0)
        {
            if (errno)
            {
                perror("PT_LWPINFO: ");
            }
            else{
                if (ret > 0)
                {
                    echo("Error getting lwp info. (%s)", strerror(ret));
                }
                else{
                    echo("Unknown error getting lwp info for stopped tracee. PT_LWPINFO returned %d", ret);
                }
            }
        }
        else{
            echo("Thread that stopped / terminated: %s (%d)", thread_info.pl_tdname, thread_info.pl_child_pid);
        }
    }
    else{
        echo("No processes wishing to report status");
    }
    
    
    // Get process' status once it has stopped
    if (WIFEXITED(game->status))
    {
        echo("Exited (sig %d)", WEXITSTATUS(game->status));
    }
    else if(WIFCONTINUED(game->status))
    {
        echo("Continued (how? we're not even using WCONTINUED)");
    }
    else if (WIFSIGNALED(game->status))
    {
        if (WCOREDUMP(game->status))
        {
            echo("Signaled w/ coredump (sig: %d)", WTERMSIG(game->status));
        }
        else{
            echo("Signaled (sig: %d)", WTERMSIG(game->status));
        }
    }
    else if (WIFSTOPPED(game->status))
    {
        echo("Stopped (sig: %d)", WSTOPSIG(game->status));
    }


    return pid;
}






// Resume execution on all threads in the traced process
static int proc_resume_threads(pid_t pid)
{
    int ret;
    int thread_count;
    lwpid_t * threads;
    errno = 0;

    echo("Resuming all threads in process %d", pid);


    // Get number of threads
    thread_count = ptrace(PT_GETNUMLWPS, pid, 0, 0);
    if (errno != 0)
    {
        perror("PT_GETNUMLWPS: ");
        return 1;
    }
    if (!thread_count)
    {
        echo("ERROR: Process has no threads. (Quoi?)");
    }



    // Allocate space for the thread ids
    threads = (lwpid_t *) malloc(sizeof(lwpid_t) * thread_count);
    if (!threads)
    {
        echo("ERROR: unable to allocate %d bytes to get thread ids.");
    }


    // Get threads themselves
    ret = ptrace(PT_GETLWPLIST, pid, (caddr_t) threads, thread_count);
    if (errno != 0)
    {
        perror("PT_GETLWPLIST: ");
        echo("Error getting thread id list. Read %d out of %d threads.", ret, thread_count);
        return 1;
    }
    else if (ret != thread_count)
    {
        echo("Error getting thread id list. Only read %d out of %d threads.", ret, thread_count);
        return 1;
    }



    
    for(int i = 0; i < thread_count; i++)
    {
        ret = ptrace(PT_RESUME, threads[i], 0, 0);
        if (errno != 0)
        {
            perror("PT_RESUME: ");
            echo("Error resuming thread #%d out of %d", i, thread_count);
            return 1;
        }
    }


    return 0;
}



// Suspend execution on all threads in the traced process
static int proc_suspend_threads(pid_t pid)
{
    int ret;
    int thread_count;
    lwpid_t * threads;
    errno = 0;

    echo("Suspending all threads in process %d", pid);


    // Get number of threads
    thread_count = ptrace(PT_GETNUMLWPS, pid, 0, 0);
    if (errno != 0)
    {
        perror("PT_GETNUMLWPS: ");
        return 1;
    }
    if (!thread_count)
    {
        echo("ERROR: Process has no threads. (Quoi?)");
    }



    // Allocate space for the thread ids
    threads = (lwpid_t *) malloc(sizeof(lwpid_t) * thread_count);
    if (!threads)
    {
        echo("ERROR: unable to allocate %d bytes to get thread ids.");
    }


    // Get threads themselves
    ret = ptrace(PT_GETLWPLIST, pid, (caddr_t) threads, thread_count);
    if (errno != 0)
    {
        perror("PT_GETLWPLIST: ");
        echo("Error getting thread id list. Read %d out of %d threads.", ret, thread_count);
        return 1;
    }
    else if (ret != thread_count)
    {
        echo("Error getting thread id list. Only read %d out of %d threads.", ret, thread_count);
        return 1;
    }


    
    for(int i = 0; i < thread_count; i++)
    {
        ret = ptrace(PT_SUSPEND, threads[i], 0, 0);
        if (errno != 0)
        {
            perror("PT_SUSPEND: ");
            echo("Error suspending thread #%d out of %d", i, thread_count);
            return 1;
        }
    }
    echo("Suspended all %d threads", thread_count);


    ret = ptrace(PT_SUSPEND, pid, 0, 0);
    if (errno != 0)
    {
        perror("PT_SUSPEND: ");
        echo("Error suspending process %d", pid);
        return 1;

    }
    return 0;
}








static int _proc_continue(pid_t pid, void * resume, int signal)
{
    int ret;
    if (!resume)
    {
        resume = (void *) 1; // (continue where it left off)
    }
    else if (resume != (void *) 1)
    {
        echo("Resuming process %d from 0x%llX", pid, resume);
    }


    if ((ret = ptrace(PT_CONTINUE, pid, resume, signal)) != 0)
    {
        echo("ERROR:\n# PT_CONTINUE failed with code %d (%s)", ret, strerror(ret));
    }

    return ret;
}

int proc_continue(pid_t pid)
{
    echo("Continuing %d", pid);
    // int ret;
    // if ((ret = kill(pid, SIGCONT)))
    // {
    //     echo("SIGCONT Error!!!");
    //     return 1;
    // }

    _proc_continue(pid, 0, SIGCONT);

    return 0;
}

int proc_stop(pid_t pid)
{
    echo("Stopping %d", pid);

    int ret;
    if ((ret = kill(pid, SIGSTOP)))
    {
        echo("SIGSTOP Error!!!");
        return 1;
    }

    // proc_suspend_threads(pid);
    return 0;
}







int proc_get_regs(pid_t pid, struct reg * regs)
{
    int ret;
    if ((ret = ptrace(PT_GETREGS, pid, (caddr_t) regs, 0)) != 0)
    {
        echo("ERROR:\n# PT_GETREGS failed with code %d (%s)\n", ret, strerror(ret));
    }

    return ret;
}


int proc_set_regs(pid_t pid, struct reg * regs)
{
    int ret;
    if ((ret = ptrace(PT_SETREGS, pid, (caddr_t) regs, 0)) != 0)
    {
        echo("ERROR:\n# PT_SETREGS failed with code %d (%s)\n", ret, strerror(ret));
    }
    
    return ret;
}


int proc_get_dbregs(pid_t pid, struct dbreg * dbregs)
{
    errno = 0;
    int ret;
    if ((ret = ptrace(PT_GETDBREGS, pid, (caddr_t) dbregs, 0)) != 0)
    {
        echo("ERROR:\n# PT_GETDBREGS failed with code %d (%d -> %s)\n", ret, errno, strerror(errno));
    }

    return ret;
}


int proc_set_dbregs(pid_t pid, struct dbreg * dbregs)
{
    errno = 0;
    int ret;
    if ((ret = ptrace(PT_SETDBREGS, pid, (caddr_t) dbregs, 0)) != 0)
    {
        echo("ERROR:\n# PT_SETDBREGS failed with code %d (%d -> %s)\n", ret, errno, strerror(errno));
    }
    
    return ret;
}






static struct dbreg regs;
static struct ptrace_lwpinfo lwpinfo;
int set_watchpoint(pid_t pid, short index, void * address, int wp_type, int wp_len)
{
    lwpid_t * lwpid_list;
    int
        ret,
        * wp_count,
        thread_count
    ;
    
    if (index > 3 || index < 0)
    {
        echo("ERROR: Invalid index supplied for watchpoint creation.");
        return 1;
    }
    



    // Get the current number of threads in the game process
    if ((thread_count = ptrace(PT_GETNUMLWPS, pid, null, 0)) == -1)
    {
        perror("PT_GETNUMLWPS: ");
        echo("ERROR:\n# PT_GETNUMLWPS failed. See error above. Aborting watchpoint creation");
        return 1;
    }
    if (thread_count < 1)
    {
        echo("ERROR: Invalid number of threads returned from PT_GETNUMLWPS. Aborting watchpoint creation.");
        return 1;
    }



    // Get the threads themselves
    lwpid_list = (lwpid_t *) malloc(thread_count * sizeof(lwpid_t));

    if ((ret = ptrace(PT_GETLWPLIST, pid, (caddr_t) lwpid_list, thread_count)) != thread_count)
    {
        perror("PT_GETLWPLIST: ");
        echo("ERROR:\n# PT_GETLWPLIST failed. Did not fill all array entries. (%d != %d)", ret, thread_count);
        goto _ret;
    }
    
    
    

    // Get the current debug register values to avoid overwriting other watchpoints
    if ((ret = ptrace(PT_GETDBREGS, pid, (caddr_t) &regs, 0)) != 0)
    {
        echo("ERROR:\n# PT_GETDBREGS failed with code %d (%s)", ret, strerror(ret));
        goto _ret;
    }


    
    // Set watch address
    regs.dr[index] = (uintptr_t) address;

    // idk what to do with DR6 at the moment, fairly sure it's written to with information on the breakpoint after it's hit
    regs.dr[6] = 0;

    // Set breakpoint flags
    regs.dr[7] &= ~DBREG_DR7_MASK(0);
    regs.dr[7] = 0x0000000000000700; // exact breakpoint enable for LE & GE
    // regs.dr[7] |= (3 << 8); // exact breakpoint enable for LE & GE
    

    wp_count = get_watchpoint_count();
    if (address)
    {
        regs.dr[7] |= DBREG_DR7_SET(index, wp_len, wp_type, DBREG_DR7_LOCAL_ENABLE | DBREG_DR7_GLOBAL_ENABLE);  // len=4 => DBREG_DR7_LEN_4 (macro encodes bits)
        (*wp_count)++;

        echo("Set Watchpoint #%d at 0x%llX, applying to %d threads.", index, address, thread_count);
    }
    else {
        regs.dr[7] |= DBREG_DR7_SET(index, 0, 0, 0);
        (*wp_count)--;
        
        echo("Removed Watchpoint #%d, applying to %d threads.", index, thread_count);
    }
    
    
    
    
    for (int i = 0; i < thread_count; i++)
    {
        if (0) // (address)
        {
            // Print thread names for shits and giggles (making sure the lwpinfo struct is all good)
            if ((ret = ptrace(PT_LWPINFO, lwpid_list[i], (caddr_t) &lwpinfo, sizeof(struct ptrace_lwpinfo))) != 0)
            {
                echo("ERROR:\n# PT_LWPINFO failed with code (%s) %d\n", strerror(ret), ret);
                break;
            }
            echo(lwpinfo.pl_tdname);
        }


        if ((ret = ptrace(PT_SETDBREGS, lwpid_list[i], (caddr_t) &regs, 0)) != 0)
        {
            echo("ERROR:\n# PT_SETDBREGS failed with code (%s) %d\n", strerror(ret), ret);
            break;
        }
    }

    
    
    // Return post-lwpid_list allocation
_ret:
    free(lwpid_list);
    return 0;
}




// Clear the watchpoint in the process at the specified index, or all watchpoints if a value less than zero is provided
int clear_watchpoint(pid_t pid, short index)
{
    int ret;
    if (index > 3)
    {
        echo("ERROR\nclear_watchpoint(): Invalid index provided for watchpoint. Must be fron 0 to 3. (Or < 0 to clear all)");
        return 1;
    }
    if (pid < 1)
    {
        echo("ERROR\nclear_watchpoint(): Invalid pid provided. (%d)", pid);
        return 1;
    }

    
    
    // Clear all watchpoints
    if (index < 0)
    {
        for (int i = 0; i < 4; i++)
        {
            ret = set_watchpoint(pid, i, NULL, 0, 0);
            if (ret != 0)
            {
                echo("ERROR\nclear_watchpoint(): error clearing watchpoint #%d", i);
            }
        }

        return ret;
    }
    


    // Set specified watchpoint
    return set_watchpoint(pid, index, NULL, 0, 0);
}




int clear_args(struct reg * regs)
{
    if (!regs)
    {
        echo("ERROR: %s->Null regs pointer provided.", __FUNCTION__);
        return 1;
    }

    regs->r_rdi = 0;
    regs->r_rsi = 0;
    regs->r_rdx = 0;
    regs->r_rcx = 0;
    regs->r_rbx = 0;
    regs->r_r8 = 0;
    regs->r_r9 = 0;

    return 0;
}





int set_args(struct reg * regs, uintptr_t rdi, uintptr_t rsi, uintptr_t rdx, uintptr_t rcx, uintptr_t rbx, uintptr_t r8, uintptr_t r9)
{
    if (!regs)
    {
        echo("ERROR: %s->Null regs pointer provided.", __FUNCTION__);
        return 1;
    }

    regs->r_rdi = rdi;
    regs->r_rsi = rsi;
    regs->r_rdx = rdx;
    regs->r_rcx = rcx;
    regs->r_rbx = rbx;
    regs->r_r8 = r8;
    regs->r_r9 = r9;

    return 0;
}







unsigned char read_byte(int pid, void * address);
unsigned long read_ulong(int pid, unsigned long * address);
int write_ulong(int pid, unsigned long * address, unsigned long value);
// Setup a function call at the specified function_address in the tracee.
int proc_call(ndgame_t * ndgame, void * function_address, unsigned long * return_val_ptr, struct reg * arg_regs)
{
    if (function_address == 0)
    {
        echo("ERROR: Null address provided for function.");
        return 1;
    }
    if (!ndgame)
    {
        echo("ERROR: Null ndgame ptr provided");
        return 1;
    }
    
    pid_t pid;
    pid = ndgame->pid;
    

    int ret;
    // I guess we can't actually read from that address? I assume we can only read after the header?
    // if ((ret = read_byte(pid, (void *) 0x400000)) != 0xCC)
    // {
    //     echo("ERROR: Unexpected value read from expected image base. Expected to find INT3 (0xCC) breakpoint instruction, read %lX instead. Aborting function call.", ret);
    //     return 1;
    // }
    
    
    
    // Back the regisers up
    struct reg og_registers;
    unsigned long error_source;
    if (proc_get_regs(pid, &og_registers))
    {
        error_source = __LINE__ - 2;
        goto _err;
    }



    
    if (!arg_regs)
    {
        arg_regs = &og_registers;
        clear_args(&og_registers);
    }
    
    
    

    // Set return address
    write_ulong(pid, (void *) (arg_regs->r_rsp -= 8), (unsigned long) 0x400000);

    // Set next instruction address
    arg_regs->r_rip = (critical_t) function_address;
    
    if (proc_set_regs(pid, arg_regs))
    {
        error_source = __LINE__ - 2;
        goto _err;
    }

    


    // Resume execution until the function returns and hits the breakpoint
    proc_continue(pid);
    proc_wait(ndgame, true);




    // Read return value
    struct reg return_regs;
    ret = proc_get_regs(pid, &return_regs);
    if (ret)
    {
        echo("Unable to get registers after function call, cannot get return value");
    }

    if (return_val_ptr > 0)
    {
        *((long long *) return_val_ptr) = return_regs.r_rax;
        echo("Function at 0x%llX returned: 0x%llX", function_address, *return_val_ptr);
    }
    else {
        echo("Function at 0x%llX returned.", function_address);
    }
    



    // Restore the initial register values 
    if (proc_set_regs(pid, &og_registers))
    {
        error_source = __LINE__ - 2;
        goto _err;
    }


    proc_continue(pid);
    return 0;

_err:
    echo("Unable to complete function call (failed at line %d)", error_source);
    return 1;
}





int old_proc_call(pid_t pid, void * function_address, unsigned long * return_val_ptr, struct reg * arg_regs)
{
    if (function_address == 0)
    {
        echo("ERROR: Null address provided for function.");
        return 1;
    }
    
    int ret;
    u_long error_source;



    // Back the regisers up
    echo("getting og regisers");
    struct reg og_registers;
    if (proc_get_regs(pid, &og_registers))
    {
        error_source = __LINE__ - 2;
        goto _err;
    }


    
    // Make a copy of the current register values if none was provided
    struct reg registers;
    if (arg_regs == nullptr)
    {
        echo("no args provided, copying og regisers to use instead");
        memcpy(&registers, &og_registers, sizeof(struct reg));
        arg_regs = &registers;
    }
    
    
    

    // Set up the function call
    unsigned long og_stack_ptr  = (long long) read_ulong(pid, (unsigned long *) arg_regs->r_rsp); // save return stack pointer

    write_ulong(pid, (void *) arg_regs->r_rsp - 8, (unsigned long) arg_regs->r_rip); // set return address

    arg_regs->r_rip = (long long) function_address; // Set next instruction address
    
    if (proc_set_regs(pid, arg_regs))
    {
        error_source = __LINE__ - 2;
        goto _err;
    }


    
    // set_watchpoint(pid, 0, (void *) og_stack_ptr, DBREG_DR7_EXEC, DBREG_DR7_LEN_1);
    // proc_continue(pid, NULL);

    //! TODO: replace this with a watchpoint
    // Step through until we return from the called function, then restore the original register values (and read the return value if a pointer to store it at was provided)
    while (1)
    {
        
        if ((ret = ptrace(PT_STEP, pid, 0, 0)) != 0)
        {
            echo("ERROR:\n# PT_STEP failed with code %d (%s) while checking rip.\n", ret, strerror(ret));
            return 1;
        }
        waitpid(pid, 0, 0);
    


        if (proc_get_regs(pid, arg_regs))
        {
            error_source = __LINE__ - 2;
            goto _err;
        }


        if (arg_regs->r_rip == (__int64_t) og_stack_ptr)
        {
            if (return_val_ptr > 0)
            {
                *((long long *) return_val_ptr) = arg_regs->r_rax;
            }
            
            // Restore the registers 
            if (proc_set_regs(pid, &og_registers))
            {
                error_source = __LINE__ - 2;
                goto _err;
            }
            
            break;
        }

        echo("%llX != %llX", arg_regs->r_rip, (__int64_t) og_stack_ptr);
    }


    return 0;

    
_err:
    echo("Unable to complete function call (failed at line %d)", error_source);
    return 1;
}




void * proc_malloc(ndgame_t * game, int amount)
{
    int ret;
    void * allocated_mem; 

    struct reg regs;
    if (proc_get_regs(game->pid, &regs))
    {
        return nullptr;
    }


    regs.r_rdi = amount;


    ret = proc_call(game, (void *) game->func_malloc, (unsigned long *) &allocated_mem, &regs);
    if (ret)
    {
        echo("ERROR: malloc call failed! (ret: %d, amount: %d)", ret, amount);
        return nullptr;
    }

    echo("allocate mem ptr: %llX", allocated_mem);

    return allocated_mem;
}


int proc_free(ndgame_t * game, void * allocated_mem)
{
    int ret;
    struct reg regs;

    regs.r_rdi = (long long) allocated_mem;

    if ((ret = proc_get_regs(game->pid, &regs)))
    {
        echo("ERROR: free proc_get_regs call failed!");
    }
    else if ((ret = proc_call(game, (void *) game->func_free, NULL, &regs)))
    {
        echo("ERROR: free call failed!");
    }

    return ret;
}












//#
//## ptrace I/O functions
//#

/*
*	Operations in piod_op.
*/
// #define PIOD_READ_D     1	     /*	Read from D space */
// #define PIOD_WRITE_D    2	     /*	Write to D space */
// #define PIOD_READ_I     3	     /*	Read from I space */
// #define PIOD_WRITE_I    4	     /*	Write to I space */

// struct ptrace_io_desc {
//     int     piod_op;	     /*	I/O operation */
//     void    *piod_offs;     /*	child offset */
//     void    *piod_addr;     /*	parent offset */
//     size_t  piod_len;	     /*	request	length */
// };

static int proc_read(pid_t pid, int len, void * source, void * dest, const char * caller)
{
    if (!source)
    {
        echo("ERROR:\n# %s->ptrace PT_IO (read)\nSource address was null (src addr: 0x%llX", caller, source);
        return 1;
    }
    if (!dest)
    {
        echo("ERROR:\n# %s->ptrace PT_IO (read)\nDestination address was null (dest addr: 0x%llX", caller, dest);
        return 1;
    }



    
    struct ptrace_io_desc iod = { PIOD_READ_D, source, dest, len };


    int ret;
    if ((ret = ptrace(PT_IO, pid, (caddr_t) &iod, 0)) != 0)
    {
        echo("ERROR:\n# %s->ptrace PT_IO (read) failed with code %d (%s)\n\n# Source address: %llX\n# Dest address: %llX\n# Function: %s\n", caller, ret, strerror(ret), source, dest, caller);
        return 1;
    }

    return 0;
}




static int proc_write(pid_t pid, int len, void * dest, void * value, const char * caller)
{
    if (!dest)
    {
        echo("ERROR:\n# %s->ptrace PT_IO (write)\nDestination address was invalid (src addr: 0x%llX", caller, dest);
        return 1;
    }
    if (!value)
    {
        echo("ERROR:\n# %s->ptrace PT_IO (write)\nValue address was invalid (val addr: 0x%llX", caller, value);
        return 1;
    }
    
    
    struct ptrace_io_desc iod = { PIOD_WRITE_D, dest, value, len };

    int ret;
    if ((ret = ptrace(PT_IO, pid, (caddr_t) &iod, 0)) != 0)
    {
        echo("ERROR:\n# %s->ptrace PT_IO (write) failed with code %d (%s)\n\n# Destination address: %llX\n# Value Pointer: %llX (value as ulong: %llX)\n# Function: %s\n", caller, ret, strerror(ret), dest, value, *(unsigned long *) value, caller);
        return 1;
    }

    return 0;
}




int read_bytes(pid_t pid, size_t len, void * address, void * buffer)
{
    return proc_read(pid, len, address, buffer, __FUNCTION__);
}



int write_bytes(pid_t pid, size_t len, void * address, void * buffer)
{
    return proc_write(pid, len, address, buffer, __FUNCTION__);
}





unsigned char read_byte(int pid, void * address)
{
    int ret;
    unsigned char data;
    errno = 0;

    if ((ret = proc_read(pid, 1, address, &data, __FUNCTION__)) != 0)
    {
        errno = ret;
        return -1;
    }

    return data;
}


int write_byte(int pid, void * address, unsigned char data)
{
    return proc_write(pid, 1, address, &data, __FUNCTION__);
}





unsigned long read_ulong(int pid, unsigned long * address)
{
    unsigned long data;

    if (proc_read(pid, 8, address, &data, __FUNCTION__))
    {
        return -1;
    }

    return data;
}


int write_ulong(int pid, unsigned long * address, unsigned long value)
{
    return proc_write(pid, 8, address, &value, __FUNCTION__);
}





unsigned int read_uint(int pid, void * address)
{
    unsigned int data;

    if (proc_write(pid, 8, address, &data, __FUNCTION__))
    {
        return -1;
    }

    return data;
}


int write_uint(int pid, void * address, unsigned int data)
{
    int ret;
    struct ptrace_io_desc iod = { 1, address, &data, 0x4 };

    if ((ret = ptrace(PT_IO, pid, (caddr_t) &iod, 0)) != 0)
    {
        echo("ERROR:\n# ptrace PT_IO failed with code: %s\n\n# Function: %s", strerror(ret), __FUNCTION__);
    }

    return ret;
}





double read_double(int pid, void * address)
{
    int ret;
    double data;
    struct ptrace_io_desc iod = { 1, (void *) address, &data, 0x8 };

    if ((ret = ptrace(PT_IO, pid, (caddr_t) &iod, 0)) != 0)
    {
        echo("ERROR:\n# ptrace PT_IO failed with code: %s\n\n# Function: %s", strerror(ret), __FUNCTION__);
        return 0xDEADBEEF;
    }

    return data;
}


int write_double(int pid, void * address, double value)
{
    return 1;
}






float read_single(int pid, unsigned long address)
{
    int ret;
    float data;
    struct ptrace_io_desc iod = { 1, (void *) address, &data, 0x4 };

    if ((ret = ptrace(PT_IO, pid, (caddr_t) &iod, 0)) != 0)
    {
        echo("ERROR:\n# ptrace PT_IO failed with code: %s\n\n# Function: %s", strerror(ret), __FUNCTION__);
        return (float) 0xDEADBEEF;
    }

    return data;
}

int write_single(int pid, unsigned long address, float value)
{
    int ret;
    float data;
    struct ptrace_io_desc iod = { 2, (void *) address, &data, 0x4 };

    if ((ret = ptrace(PT_IO, pid, (caddr_t) &iod, 0)) != 0)
    {
        echo("ERROR:\n# ptrace PT_IO failed with code: %s\n\n# Function: %s", strerror(ret), __FUNCTION__);
        return 1;
    }

    return 0;
}





// Read a null-terminated string of characters from the traced process' memory from the provided address.
// Returns the amount of characters written, or 0 if there were any errors such as a null string or buffer pointer, or an I/O issue
int read_string(int pid, const char * address, char * buffer)
{
    if (address == 0)
    {
        echo("null string pointer provided.");
        return -1;
    }
    

    if (read_byte(pid, (void *) address) <= 0) 
    {
        echo("Empty string at address 0x%llX.", (unsigned long long) address);
        return 0;
    }
    
 
    // Get string length
    int len = 0;
    for (; read_byte(pid, (void *) address + len) != 0; ++len);
    
    
    // Read string for len bytes
    for(long long i = 0; i < len; ++i)
    {
        *(char *) (buffer + i) = read_byte(pid, (void *) address + i);
    }
    *(char *) (buffer + len) = 0; // terminate the string

    return len;
}



// Write a null-terminated string of characters to the traced process' memory.
// Returns the amount of characters written, or -1 if a null pointer was provided for either the source or the destination
int write_string(int pid, void * address, const char * str)
{
    if (!address) {
        echo("null destination string pointer provided.");
        return -1;
    }

    if (!str) {
        echo("null source string pointer provided.");
        return -1;
    }

    if (!*str) 
    {
        echo("Empty string at address 0x%llX.", (unsigned long long) address);
        return -1;
    }
    
 
    // Get string length
    int len = 0;
    int written = 0;
    while (*(str + ++len) != 0);
    
    for(long long i = 0; i < len; ++i)
    {
        if (!write_byte(pid, (void *) address + i, *(char *) (str + i)))
        {
            ++written;
        }
    }
    write_byte(pid, (void *) address + len, 0);

    return written;
}

