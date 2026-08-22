#include "inc/include.h"
#include "inc/dmenu.h"
#include "inc/memory.h"

#define _SHELLCORE_AUTHID 0x4800000000010003L








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








int attach(ndgame_t * ndgame)
{
    int ret;
    errno = 0;

    
    ret = set_authid_for_ptrace();
    if (errno)
    {
        echo("ERROR: Unable to set ucred authid for ptrace (Error: %s)", strerror(errno));
        return 1;
    }


    if ((ret = ptrace(PT_ATTACH, ndgame->pid, 0, 0)) != 0)
    {
        notify("PT_ATTACH ERROR: %s\n", strerror(ret));
        
        if(kill(ndgame->pid, SIGCONT))
        {
            echo("WARNING: Unable to resume process through SIGCONT");
        }
    }
    
    echo("Attached to pid %d", ndgame->pid);
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
pid_t proc_wait(ndgame_t * ndgame, int wait)
{
    // Set options for wait pid
    int options;
    options = WTRAPPED | WEXITED | WUNTRACED;
    
    if (!wait)
    {
        options |= WNOHANG;
        echo("Waiting for pid %d w/ NOHANG", ndgame->pid);
    }
    else{
        echo("Waiting for pid %d.", ndgame->pid);
    }
    
    

    // Wait for a child like EDP at the mall
    errno = 0;
    pid_t returned_pid;
    returned_pid = waitpid(-1, &ndgame->status, options); // WTRAPPED and WEXITED are set implicitly for waitpid, but meh
    if (returned_pid == -1)
    {
        if (errno == ECHILD)
        {
            echo("No children not previously awaited");
        }
        else {
            echo("ERROR: waitpid returned error: %d->%s", errno, strerror(errno));
        }
        
        return -1;
    }
    
    
    echo("Process %d stopped with status: 0x%llX.", returned_pid, ndgame->status);


    if (!returned_pid)
    {
        if (wait)
        {
            echo("ERROR: NOHANG was not specified, but waitpid returned 0 anyway.");
            errno = 1;
            return -1;
        }

        echo("No processes wishing to report status.");
        return 0;
    }
    
    // Get process' status once it has stopped
    if (WIFEXITED(ndgame->status))
    {
        echo("Exited (sig %d)", WEXITSTATUS(ndgame->status));
    }
    else if(WIFCONTINUED(ndgame->status))
    {
        echo("Continued (how? we're not even using WCONTINUED)");
    }
    else if (WIFSIGNALED(ndgame->status))
    {
        if (WCOREDUMP(ndgame->status))
        {
            echo("Signaled w/ coredump (sig: %d)", WTERMSIG(ndgame->status));
        }
        else{
            echo("Signaled (sig: %d)", WTERMSIG(ndgame->status));
        }
    }
    else if (WIFSTOPPED(ndgame->status))
    {
        echo("Stopped (sig: %d)", WSTOPSIG(ndgame->status));
    }






    // Get thread info
    int ret;
    errno = 0;
    struct ptrace_lwpinfo thread_info;
    ret = ptrace(PT_LWPINFO, ndgame->pid, (caddr_t) &thread_info, sizeof(struct ptrace_lwpinfo));
    if (ret != 0)
    {
        if (errno)
        {
            perror("PT_LWPINFO: ");
        }
        else{
            if (ret)
            {
                echo("Error getting lwp info. (%s)", strerror(ret));
            }
            else{
                echo("Unknown error getting lwp info for stopped tracee. PT_LWPINFO returned %d", ret);
            }
        }
    }
    else{
        echo("Thread that stopped / terminated: %s (%d:%d)", thread_info.pl_tdname, returned_pid, thread_info.pl_child_pid);
    }


    return returned_pid;
}








// Resume execution on all threads in the traced process
static int proc_resume_threads(ndgame_t * ndgame)
{
    int ret;
    int thread_count;
    pid_t pid;
    pid = ndgame->pid;
    lwpid_t * threads;
    
    echo("Resuming all threads in process %d", pid);
    
    
    // Get number of threads1
    errno = 0;
    thread_count = ptrace(PT_GETNUMLWPS, pid, 0, 0);
    __rerr(thread_count < 0, "Error getting thread id list. PT_GETNUMLWPS returned a negative value.", 1);
    __rerr(thread_count == 0, "Error getting thread id list. Process contains no threads??", 1);





    // Allocate space for the thread ids
    threads = (lwpid_t *) malloc(sizeof(lwpid_t) * thread_count);
    if (!threads)
    {
        echo("ERROR: unable to allocate %d bytes to get thread ids.");
    }


    // Get threads themselves
    errno = 0;
    ret = ptrace(PT_GETLWPLIST, pid, (caddr_t) threads, thread_count);
    __rerr(ret != thread_count, strbuild("Error getting thread id list. Only read %d out of %d threads.", ret, thread_count), 1);
    __rerr(!ret, "Error getting thread id list. (PT_GETLWPLIST returned error)", 1);




    errno = 0;
    for(int i = 0; i < thread_count; i++)
    {
        ret = ptrace(PT_RESUME, threads[i], 0, 0);
        __rerr(ret, strbuild("Error resuming thread #%d out of %d", i, thread_count), 1);
    }


    return ret != 0;
}




// Suspend execution on all threads in the traced process
static int proc_suspend_threads(ndgame_t * ndgame)
{
    int ret;
    int thread_count;
    lwpid_t * threads;
    errno = 0;

    pid_t pid;
    pid = ndgame->pid;
    echo("Suspending all threads in process %d", pid);


    // Get number of threads
    thread_count = ptrace(PT_GETNUMLWPS, pid, 0, 0);
    __rerr(thread_count < 0, "Error getting thread id list. PT_GETNUMLWPS returned a negative value.", 1);
    __rerr(thread_count == 0, "Error getting thread id list. Process contains no threads??", 1);


    // Allocate space for the thread ids
    threads = (lwpid_t *) malloc(sizeof(lwpid_t) * thread_count);
    if (!threads)
    {
        echo("ERROR: unable to allocate %d bytes to get thread ids.");
    }


    // Get threads themselves
    ret = ptrace(PT_GETLWPLIST, pid, (caddr_t) threads, thread_count);
    __rerr(ret != thread_count, strbuild("Error getting thread id list. Only read %d out of %d threads.", ret, thread_count), 1);
    __rerr(!ret, "Error getting thread id list. (PT_GETLWPLIST returned error)", 1);


    
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
    if (ret || errno)
    {
        perror("PT_SUSPEND: ");
        echo("Error suspending process %d", pid);
    }

    return ret != 0;
}








int proc_continue(ndgame_t * ndgame)
{
    echo("Continuing %d", ndgame->pid);
    
    int ret;
    ret = ptrace(PT_CONTINUE, ndgame->pid, (caddr_t) 1, SIGCONT);
    if (ret)
    {
        echo("ERROR:\n# PT_CONTINUE failed with code %d (%s)", ret, strerror(ret));
        __err(1, nullptr);
    }

    // ret = proc_resume_threads(ndgame->pid);
    // __err(ret != 0, strbuild("proc_resume_threads failed", ndgame->pid));
    return ret != 0;
}




int proc_stop(ndgame_t * ndgame)
{
    echo("Stopping %d", ndgame->pid);

    int ret;
    ret = kill(ndgame->pid, SIGSTOP);
    __err(ret != 0, strbuild("kill(%d, SIGSTOP) failed", ndgame->pid));

    // ret = proc_suspend_threads(ndgame->pid);
    // __err(ret != 0, strbuild("proc_suspend_threads failed", ndgame->pid));
    return ret != 0;
}







int proc_get_regs(ndgame_t * ndgame, struct reg * regs)
{
    int ret;
    if ((ret = ptrace(PT_GETREGS, ndgame->pid, (caddr_t) regs, 0)) != 0)
    {
        echo("ERROR:\n# PT_GETREGS failed with code %d (%s)\n", ret, strerror(ret));
    }

    return ret != 0;
}


int proc_set_regs(ndgame_t * ndgame, struct reg * regs)
{
    int ret;
    if ((ret = ptrace(PT_SETREGS, ndgame->pid, (caddr_t) regs, 0)) != 0)
    {
        echo("ERROR:\n# PT_SETREGS failed with code %d (%s)\n", ret, strerror(ret));
        if (errno)
        {
            perror("PT_SETREGS: ");
        }
    }
    
    return ret != 0;
}


int proc_get_dbregs(ndgame_t * ndgame, struct dbreg * dbregs)
{
    errno = 0;
    int ret;
    if ((ret = ptrace(PT_GETDBREGS, ndgame->pid, (caddr_t) dbregs, 0)) != 0)
    {
        echo("ERROR:\n# PT_GETDBREGS failed with code %d (%d -> %s)\n", ret, errno, strerror(errno));
    }

    return ret != 0;
}


int proc_set_dbregs(ndgame_t * ndgame, struct dbreg * dbregs)
{
    errno = 0;
    int ret;
    if ((ret = ptrace(PT_SETDBREGS, ndgame->pid, (caddr_t) dbregs, 0)) != 0)
    {
        echo("ERROR:\n# PT_SETDBREGS failed with code %d (%d -> %s)\n", ret, errno, strerror(errno));
    }
    
    return ret != 0;
}







//! TODO: Replace these
static struct dbreg regs;
static struct ptrace_lwpinfo lwpinfo;
int set_watchpoint(ndgame_t * ndgame, short index, void * address, int wp_type, int wp_len)
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
    pid_t pid;
    pid = ndgame->pid;
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

        if (0)
        echo("Set Watchpoint #%d at 0x%llX, applying to %d threads.", index, address, thread_count);
    }
    else {
        regs.dr[7] |= DBREG_DR7_SET(index, 0, 0, 0);
        (*wp_count)--;
     
        if (0)
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
int clear_watchpoint(ndgame_t * ndgame, short index)
{
    int ret;
    if (index > 3)
    {
        echo("ERROR\nclear_watchpoint(): Invalid index provided for watchpoint. Must be fron 0 to 3. (Or < 0 to clear all)");
        return 1;
    }
    pid_t pid;
    pid = ndgame->pid;
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
            ret = set_watchpoint(ndgame, i, NULL, 0, 0);
            if (ret != 0)
            {
                echo("ERROR\nclear_watchpoint(): error clearing watchpoint #%d", i);
            }
        }

        return ret;
    }
    


    // Set specified watchpoint
    return set_watchpoint(ndgame, index, NULL, 0, 0);
}




int clear_args(ndgame_t * ndgame, struct reg * regs)
{
    if (!regs)
    {
        echo("ERROR: %s->Null regs pointer provided.", __FUNCTION__);
        return 1;
    }


    // Get the current register values to ensure critical registers like RSP are still filled
    int ret;
    ret = proc_get_regs(ndgame, regs);
    if (ret)
    {
        echo("ERROR: %s->proc_get_regs failed, unable to get register values when clearing args. clearing main registers anyway, but returning error", __FUNCTION__);
    }



    regs->r_rdi = 0;
    regs->r_rsi = 0;
    regs->r_rdx = 0;
    regs->r_rcx = 0;
    regs->r_rbx = 0;
    regs->r_r8 = 0;
    regs->r_r9 = 0;

    return ret != 0;
}




int set_args(ndgame_t * ndgame, struct reg * regs, uintptr_t rdi, uintptr_t rsi, uintptr_t rdx, uintptr_t rcx, uintptr_t rbx, uintptr_t r8, uintptr_t r9)
{
    if (!regs)
    {
        echo("ERROR: %s->Null regs pointer provided.", __FUNCTION__);
        return 1;
    }


    // Get the current register values to ensure critical registers like RSP are still filled
    int ret;
    ret = proc_get_regs(ndgame, regs);
    if (ret)
    {
        echo("ERROR: %s->proc_get_regs failed, unable to get register values when clearing args. clearing main registers anyway, but returning error", __FUNCTION__);
    }



    regs->r_rdi = rdi;
    regs->r_rsi = rsi;
    regs->r_rdx = rdx;
    regs->r_rcx = rcx;
    regs->r_rbx = rbx;
    regs->r_r8 = r8;
    regs->r_r9 = r9;

    return ret != 0;
}








byte read_byte(int pid, byte * address);
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
    
    // Construct reused error message
    char errmsg[0x200];
    strncpy(errmsg, (const char *) strbuild("Unable to complete function call to 0x%llX", function_address), sizeof(errmsg));

    // Check for a null or invalid ndgame_t pointer
    if (!ndgame)
    {
        echo("ERROR: %s->Null ndgame ptr provided", __FUNCTION__);
        __rerr(1, errmsg, 1);
    }

    
    
    //! I guess we can't actually read from that address? I assume we can only read after the header?
    // int ret;
    // ret = read_byte(ndgame->pid, (void *) 0x400000);
    // __rerr(ret != 0xCC || errno, errmsg, 1);
    

    
    // Back the regisers up
    int ret;
    struct reg og_registers;
    ret = proc_get_regs(ndgame, &og_registers);
    __rerr(ret, errmsg, 1);

    
    
    // Default to current register values
    if (!arg_regs)
    {
        echo("No args provided, using og_registers");
        arg_regs = &og_registers;
    }
    
    
    

    // Set return address
    write_ulong(ndgame->pid, (void *) (arg_regs->r_rsp -= 8), (unsigned long) 0x400000);
    
    // Set next instruction address
    arg_regs->r_rip = (critical_t) function_address;
    
    ret = proc_set_regs(ndgame, arg_regs);
    __rerr(ret, errmsg, 1);

    
    
    
    // Resume execution until the function returns and hits the breakpoint
    ret = proc_continue(ndgame);
    __rerr(ret, errmsg, 1);

    ret = proc_wait(ndgame, true);
    __rerr(!ret, errmsg, 1);





    if (return_val_ptr > 0)
    {
        // Read return value
        struct reg return_regs;
        ret = proc_get_regs(ndgame, &return_regs);
        __rerr(ret, errmsg, 1);

        *((long long *) return_val_ptr) = return_regs.r_rax;

        echo("Function at 0x%llX returned: 0x%llX", function_address, *return_val_ptr);
    }
    else {
        echo("Function at 0x%llX returned.", function_address);
    }
    



    // Restore the initial register values
    ret = proc_set_regs(ndgame, &og_registers);
    __rerr(ret, errmsg, 1);
    
    return 0;
}








void * proc_malloc(ndgame_t * ndgame, size_t amount)
{
    struct reg regs;
    __rerr(proc_get_regs(ndgame, &regs), nullptr, nullptr);


    // set malloc arguments
    __rerr(set_args(ndgame, &regs, amount, 0, 0, 0, 0, 0, 0), null, nullptr);


    // call malloc
    void * mem;
    __rerr(proc_call(ndgame, ndgame->func_malloc, (unsigned long *) &mem, &regs), null, (void *) nullptr);

    return mem;
}


int proc_free(ndgame_t * ndgame, void * allocated_mem)
{
    struct reg regs;

    regs.r_rdi = (long long) allocated_mem;

    __rerr(proc_get_regs(ndgame, &regs), "ERROR: free proc_get_regs call failed!", 1);
    __rerr(proc_call(ndgame, (void *) ndgame->func_free, NULL, &regs), "ERROR: Call to tracee's \"free\" failed", 1);
    
    return 0;
}







int patch_process(ndgame_t * ndgame, size_t patch_count, byte * patch_addresses[], const byte * patch_data[], size_t patch_sizes[])
{
    __rerr(1, "Unimplimented function called prematurely.", 1);
    exit(1);


    __rerr(!ndgame, "Invalid ndgame pointer provided.", 1);
    __rerr(!patch_count, "WARNING: Patch count was zero.", 0);
    __rerr(!(byte *) patch_data, "Invalid patch data array pointer provided.", 1);

    proc_wait(ndgame, false);

    int ret;
    echo("Writing %d patches to process %d...", patch_count, ndgame->pid);
    for (int i = 0, j = 0; i < patch_count; i++, j = 0)
    {
        while (j < patch_sizes[i])
        {
            ret = write_byte(ndgame->pid, patch_addresses[i] + j, *(patch_data[i] + j));
            __rerr(ret, strbuild("Error writing byte @0x%X to 0x%llX", *(patch_data[i] + j), patch_addresses[i] + j), 1);

            ++j;
        }
    }


    byte chk;
    echo("Verifying patches were written to %d...", ndgame->pid);
    for (int i = 0, j = 0; i < patch_count; i++, j = 0)
    {
        while (j < patch_sizes[i])
        {
            chk = read_byte(ndgame->pid, patch_addresses[i] + j);
            __rerr(errno, strbuild("Error reading expected byte @0x%X from 0x%llX", *(patch_data[i] + j), patch_addresses[i] + j), 1);

            if (chk != *(patch_data[i] + j))
            {
                echo("Patch %d was not applied correctly (fail address: 0x%llX, expected value: 0x%X, read value: 0x%X)", patch_addresses[i] + j, *(patch_data[i] + j), chk);
                break;
            }


            ++j;
        }
    }
}

