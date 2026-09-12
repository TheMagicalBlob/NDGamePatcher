#include "inc/include.h"
#include "inc/memory.h"
// #include "inc/dmenu.h"
#ifdef DEBUG
#include "inc/debug.h"
#endif

extern ndgame_t * game;





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








int attach(pid_t pid)
{
    int ret;
    errno = 0;

    
    ret = set_authid_for_ptrace();
    if (errno)
    {
        echo("ERROR: Unable to set ucred authid for ptrace (Error: %s)", strerror(errno));
        return 1;
    }


    if ((ret = ptrace(PT_ATTACH, pid, 0, 0)) != 0)
    {
        notify("PT_ATTACH ERROR: %s\n", strerror(ret));
        
        if(kill(pid, SIGCONT))
        {
            echo("WARNING: Unable to resume process through SIGCONT");
        }
    }
    
    game->pid = pid;
    echo("Attached to pid %d", pid);
    return ret;
}








pid_t proc_wait(int wait);
// //!
int detach(const char * from)
{
    if (game->pid < 1)
    {
        echo("%s: Invalid pid provided (%d).", __FUNCTION__, game->pid);
        return 1;
    }
    int ret;
    pid_t pid;

    echo("Detaching from process %d (%s)", game->pid, from);


    pid = game->pid;


    kill(pid, SIGSTOP);
    
    ret = proc_wait(false);
    __err(ret != 0, "Error waiting for process, but we're exiting anyway");
    bzero(game, sizeof(ndgame_t));


    ret = ptrace(PT_DETACH, pid, 0, 0);
    __err(ret != 0, "Error detaching from game process.");

    kill(pid, SIGCONT);

    return ret;
}






// Wait for the secified process to either be stopped, or exit
// If waitpid experiences an error, returns -1
// otherwise, returns the result of WIFEXITED
pid_t proc_wait(int wait)
{
    // Set options for wait pid
    int options;
    options = WTRAPPED | WEXITED | WUNTRACED;
    
    if (!wait)
    {
        options |= WNOHANG;
        echo("Waiting for pid %d w/ NOHANG", game->pid);
    }
    else{
        echo("Waiting for pid %d.", game->pid);
    }
    
    

    // Wait for a child like EDP at the mall
    errno = 0;
    pid_t returned_pid;
    returned_pid = waitpid(game->pid, &game->status, options); // WTRAPPED and WEXITED are set implicitly for waitpid, but meh
    __rerrv(returned_pid == -1, "", -1);
    
    


    if (returned_pid == 0)
    {
        __rerrv(wait != 0, "ERROR: NOHANG was not specified, but waitpid returned 0 anyway.", -1);

        echo("No processes wishing to report status.");
        return returned_pid;
    }
    
    
    echo("Process %d stopped with status: 0x%llX.", returned_pid, game->status);

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






    // Get thread info
    int ret;
    errno = 0;
    struct ptrace_lwpinfo thread_info;
    ret = ptrace(PT_LWPINFO, game->pid, (caddr_t) &thread_info, sizeof(struct ptrace_lwpinfo));
    __rerrv(ret != 0, strbufbuild("ERROR: unable to get lwp info for stopped process in pid %d", game->pid), -1);

    echo("Thread that stopped / terminated: %s (%d:%d)", thread_info.pl_tdname, returned_pid, thread_info.pl_child_pid);
    return returned_pid;
}








// Resume execution on all threads in the traced process
static int proc_resume_threads()
{
    int ret;
    int thread_count;
    pid_t pid;
    pid = game->pid;
    lwpid_t * threads;
    
    echo("Resuming all threads in process %d", pid);
    
    
    // Get number of threads1
    errno = 0;
    thread_count = ptrace(PT_GETNUMLWPS, pid, 0, 0);
    __rerrv(thread_count < 0, "Error getting thread id list. PT_GETNUMLWPS returned a negative value.", 1);
    __rerrv(thread_count == 0, "Error getting thread id list. Process contains no threads??", 1);





    // Allocate space for the thread ids
    threads = (lwpid_t *) malloc(sizeof(lwpid_t) * thread_count);
    if (!threads)
    {
        echo("ERROR: unable to allocate %d bytes to get thread ids.");
    }


    // Get threads themselves
    errno = 0;
    ret = ptrace(PT_GETLWPLIST, pid, (caddr_t) threads, thread_count);
    __rerrv(ret != thread_count, strbufbuild("Error getting thread id list. Only read %d out of %d threads.", ret, thread_count), 1);
    __rerrv(!ret, "Error getting thread id list. (PT_GETLWPLIST returned error)", 1);




    errno = 0;
    for(int i = 0; i < thread_count; i++)
    {
        ret = ptrace(PT_RESUME, threads[i], 0, 0);
        __rerrv(ret, strbufbuild("Error resuming thread #%d out of %d", i, thread_count), 1);
    }


    return ret != 0;
}




// Suspend execution on all threads in the traced process
static int proc_suspend_threads()
{
    int ret;
    int thread_count;
    lwpid_t * threads;
    errno = 0;

    pid_t pid;
    pid = game->pid;
    echo("Suspending all threads in process %d", pid);


    // Get number of threads
    thread_count = ptrace(PT_GETNUMLWPS, pid, 0, 0);
    __rerrv(thread_count < 0, "Error getting thread id list. PT_GETNUMLWPS returned a negative value.", 1);
    __rerrv(thread_count == 0, "Error getting thread id list. Process contains no threads??", 1);


    // Allocate space for the thread ids
    threads = (lwpid_t *) malloc(sizeof(lwpid_t) * thread_count);
    if (!threads)
    {
        echo("ERROR: unable to allocate %d bytes to get thread ids.");
    }


    // Get threads themselves
    ret = ptrace(PT_GETLWPLIST, pid, (caddr_t) threads, thread_count);
    __rerrv(ret != thread_count, strbufbuild("Error getting thread id list. Only read %d out of %d threads.", ret, thread_count), 1);
    __rerrv(!ret, "Error getting thread id list. (PT_GETLWPLIST returned error)", 1);


    
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








int proc_continue()
{
    echo("Continuing %d", game->pid);
    
    int ret;
    ret = ptrace(PT_CONTINUE, game->pid, (caddr_t) 1, SIGCONT);
    if (ret)
    {
        echo("ERROR:\n# PT_CONTINUE failed with code %d (%s)", ret, strerror(ret));
        __err(1, (char *) nullptr);
    }

    // ret = proc_resume_threads(game->pid);
    // __err(ret != 0, strbufbuild("proc_resume_threads failed", game->pid));
    return ret != 0;
}




int proc_stop()
{
    echo("Stopping %d", game->pid);

    int ret;
    ret = kill(game->pid, SIGSTOP);
    __err(ret != 0, strbufbuild("kill(%d, SIGSTOP) failed", game->pid));

    // ret = proc_suspend_threads(game->pid);
    // __err(ret != 0, strbufbuild("proc_suspend_threads failed", game->pid));
    return ret != 0;
}







int proc_get_regs(struct reg * regs)
{
    int ret;
    ret = ptrace(PT_GETREGS, game->pid, (caddr_t) regs, 0);

    __rerrv(ret != 0, strbufbuild("ERROR:\n# PT_GETREGS failed with code %d (%s)\n", ret, strerror(ret)), 1);

    #ifdef VERBOSEDEBUG
    echo("");
    echo("# Registers (READ):");
    echo("# rax: %llX  rbx: %llX", regs->r_rax, regs->r_rbx);
    echo("# rcx: %llX  rdx: %llX", regs->r_rcx, regs->r_rdx);
    echo("# rsi: %llX  rdi: %llX", regs->r_rsi, regs->r_rdi);
    echo("# rbp: %llX  rsp: %llX", regs->r_rbp, regs->r_rsp);
    echo("# r8:  %llX  r9:  %llX", regs->r_r8, regs->r_r9);
    echo("# r10: %llX  r11: %llX", regs->r_r10, regs->r_r11);
    echo("# r12: %llX  r13: %llX", regs->r_r12, regs->r_r13);
    echo("# r14: %llX  r15: %llX", regs->r_r14, regs->r_r15);
    echo("# rip: %llX  eflags: %llX", regs->r_rip, regs->r_rflags);
    echo("# trapno:  %llX  err: %d", regs->r_trapno, regs->r_err);
    echo("");
    #endif
    return 0;
}


int proc_set_regs(struct reg * regs)
{
    int ret;
    if ((ret = ptrace(PT_SETREGS, game->pid, (caddr_t) regs, 0)) != 0)
    {
        echo("ERROR:\n# PT_SETREGS failed with code %d (%s)\n", ret, strerror(ret));
        if (errno)
        {
            perror("PT_SETREGS: ");
        }
    }
    
    
    #ifdef VERBOSEDEBUG
    echo("");
    echo("# Registers (WRITTEN):");
    echo("# rax: %llX  rbx: %llX", regs->r_rax, regs->r_rbx);
    echo("# rcx: %llX  rdx: %llX", regs->r_rcx, regs->r_rdx);
    echo("# rsi: %llX  rdi: %llX", regs->r_rsi, regs->r_rdi);
    echo("# rbp: %llX  rsp: %llX", regs->r_rbp, regs->r_rsp);
    echo("# r8:  %llX  r9:  %llX", regs->r_r8, regs->r_r9);
    echo("# r10: %llX  r11: %llX", regs->r_r10, regs->r_r11);
    echo("# r12: %llX  r13: %llX", regs->r_r12, regs->r_r13);
    echo("# r14: %llX  r15: %llX", regs->r_r14, regs->r_r15);
    echo("# rip: %llX  eflags: %llX", regs->r_rip, regs->r_rflags);
    echo("# trapno:  %llX  err: %d", regs->r_trapno, regs->r_err);
    echo("");
    #endif
    return ret != 0;
}


int proc_get_dbregs(struct dbreg * dbregs)
{
    errno = 0;
    int ret;
    if ((ret = ptrace(PT_GETDBREGS, game->pid, (caddr_t) dbregs, 0)) != 0)
    {
        echo("ERROR:\n# PT_GETDBREGS failed with code %d (%d -> %s)\n", ret, errno, strerror(errno));
    }

    return ret != 0;
}


int proc_set_dbregs(struct dbreg * dbregs)
{
    errno = 0;
    int ret;
    if ((ret = ptrace(PT_SETDBREGS, game->pid, (caddr_t) dbregs, 0)) != 0)
    {
        echo("ERROR:\n# PT_SETDBREGS failed with code %d (%d -> %s)\n", ret, errno, strerror(errno));
    }
    
    return ret != 0;
}







//! TODO: Replace these
static struct dbreg regs;
static struct ptrace_lwpinfo lwpinfo;
int set_watchpoint(short index, void * address, int wp_type, int wp_len)
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
    pid = game->pid;
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
int clear_watchpoint(short index)
{
    int ret;
    if (index > 3)
    {
        echo("ERROR\nclear_watchpoint(): Invalid index provided for watchpoint. Must be fron 0 to 3. (Or < 0 to clear all)");
        return 1;
    }
    pid_t pid;
    pid = game->pid;
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
            ret = set_watchpoint(i, NULL, 0, 0);
            if (ret != 0)
            {
                echo("ERROR\nclear_watchpoint(): error clearing watchpoint #%d", i);
            }
        }

        return ret;
    }
    


    // Set specified watchpoint
    return set_watchpoint(index, NULL, 0, 0);
}








int proc_clear_args(struct reg * regs)
{
    if (!regs)
    {
        echo("ERROR: %s->Null regs pointer provided.", __FUNCTION__);
        return 1;
    }


    // Get the current register values to ensure critical registers like RSP are still filled
    int ret;
    ret = proc_get_regs(regs);
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




int proc_set_args(struct reg * regs, uintptr_t rdi, uintptr_t rsi, uintptr_t rdx, uintptr_t rcx, uintptr_t rbx, uintptr_t r8, uintptr_t r9)
{
    if (!regs)
    {
        echo("ERROR: %s->Null regs pointer provided.", __FUNCTION__);
        return 1;
    }


    // Get the current register values to ensure critical registers like RSP are still filled
    int ret;
    ret = proc_get_regs(regs);
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








// Setup a function call at the specified function_address in the tracee.
int proc_call(void * function_address, unsigned long * return_val_ptr, struct reg * arg_regs)
{
    if (function_address == 0)
    {
        echo("ERROR: Null address provided for function.");
        return 1;
    }

    
    // Construct reused error message
    char errmsg[0x200];
    strncpy(errmsg, (const char *) strbufbuild("Unable to complete function call to 0x%llX", function_address), sizeof(errmsg));

    
    
    //! I guess we can't actually read from that address? Can we only read after the header?
    // int ret;
    // ret = read_byte(game->pid, (void *) 0x400000);
    // __rerrv(ret != 0xCC, errmsg, 1);
    

    
    // Back the regisers up
    int ret;
    struct reg og_registers;
    ret = proc_get_regs(&og_registers);
    __rerrv(ret, errmsg, 1);

    
    
    // Default to current register values
    if (!arg_regs)
    {
        echo("No args provided, using og_registers");
        arg_regs = &og_registers;
    }
    
    
    

    // Set return address
    ret = write_ulong(game->pid, (void *) (arg_regs->r_rsp -= 8), (unsigned long) 0x400000);
    __rerrv(ret != 0, "ERROR: Failed setting return address, aborting function call", 1);
    
    // Set next instruction address
    arg_regs->r_rip = (critical_t) function_address;
    
    ret = proc_set_regs(arg_regs);
    __rerrv(ret, errmsg, 1);

    
    
    
    // Resume execution until the function returns, hitting the breakpoint present at the start of every ps5 elf
    struct reg return_regs;
    ret = proc_continue();   // Resume process
    __rerrv(ret, errmsg, 1);

    ret = proc_wait(true);   // Wait for stop
    __rerrv(!ret, errmsg, 1);

    ret = proc_get_regs(&return_regs);
    __rerrv(ret, errmsg, 1);

    if (return_regs.r_rip != 0x400001)
    {
        echo("WARNING: Process stopped before returning from call to 0x%llX. (Next instruction when stopped: 0x%llX)", function_address, return_regs.r_rip);
        goto _return; // Repeat if it's stopped for an unrelated reason (//! handle that)
    }

    
    
    if (return_val_ptr > 0)
    {
        // Read return value
        *(long long *) return_val_ptr = return_regs.r_rax;
        
        echo("Function at 0x%llX returned: 0x%llX", function_address, *return_val_ptr);
    }
    else {
        echo("Function at 0x%llX returned.", function_address);
    }
    



_return:
    // Restore the initial register values
    ret = proc_set_regs(&og_registers);
    __rerrv(ret, errmsg, 1);
    
    return 0;
}








void * proc_malloc(size_t amount)
{
    int ret;         // Return status for function calls
    void * mem;      // Allocated memory pointer
    struct reg regs; // Register struct for call args


    // set malloc arguments
    ret = proc_set_args(&regs, amount, 0, 0, 0, 0, 0, 0);
    __rerrv(ret, nullstr, nullptr);

    
    echo("Calling 0x%llX", game->addrs.func.malloc);
    // call malloc
    ret = proc_call(game->addrs.func.malloc, (unsigned long *) &mem, &regs);
    echo("call done");
    __rerrv(ret, nullstr, nullptr);

    return mem;
}




int proc_free(void * allocated_mem)
{
    int ret;         // Return status for function calls
    struct reg regs; // Register struct for call args


    // set free arguments
    ret = proc_set_args(&regs, (uintptr_t) allocated_mem, 0, 0, 0, 0, 0, 0);
    __rerrv(ret, "ERROR: free proc_set_args call failed!", 1);

    // Call free
    ret = proc_call((void *) game->addrs.func.free, NULL, &regs);
    __rerrv(ret, "ERROR: Call to tracee's \"free\" failed", 1);
    
    return 0;
}







int patch_process(size_t patch_count, byte * patch_addresses[], const byte * patch_data[], size_t patch_sizes[])
{
    __rerrv(1, "Unfinished function called prematurely.", 1);
    ((void * (*)()) NULL)();
    echo("We still fucking goin");
    exit(1);

    __rerrv(!game, "Invalid game pointer provided.", 1);
    __rerrv(!patch_count, "WARNING: Patch count was zero.", 0);
    __rerrv(!(byte *) patch_data, "Invalid patch data array pointer provided.", 1);

    proc_wait(false);

    int ret;
    echo("Writing %d patches to process %d...", patch_count, game->pid);
    for (int i = 0, j = 0; i < patch_count; i++, j = 0)
    {
        while (j < patch_sizes[i])
        {
            ret = write_byte(game->pid, patch_addresses[i] + j, *(patch_data[i] + j));
            __err(ret, strbufbuild("Error writing byte @0x%X to 0x%llX", *(patch_data[i] + j), patch_addresses[i] + j));

            ++j;
        }
    }


    byte chk;
    echo("Verifying patches were written to %d...", game->pid);
    for (int i = 0, j = 0; i < patch_count; i++, j = 0)
    {
        while (j < patch_sizes[i])
        {
            chk = read_byte(game->pid, patch_addresses[i] + j);
            __rerrv(errno, strbufbuild("Error reading expected byte @0x%X from 0x%llX", *(patch_data[i] + j), patch_addresses[i] + j), 1);

            if (chk != *(patch_data[i] + j))
            {
                echo("Patch %d was not applied correctly (fail address: 0x%llX, expected value: 0x%X, read value: 0x%X)", patch_addresses[i] + j, *(patch_data[i] + j), chk);
                break;
            }


            ++j;
        }
    }
}

