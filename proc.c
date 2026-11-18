#include "inc/include.h"
#include "inc/memory.h"
#include "inc/dmenu.h"

extern NdGame_t * game;





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
    bzero(game, sizeof(NdGame_t));


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
    
    if (!wait) {
        options |= WNOHANG; // Add NoHang option to avoid hanging
#if VERBOSITY > 1
        echo("Waiting for pid %d w/ NOHANG", game->pid);
#endif
    }
#if VERBOSITY > 1
    else{
        echo("Waiting for pid %d.", game->pid);
    }
#endif
    

    // Wait for a child like EDP at the mall
    errno = 0;
    pid_t returned_pid;
    returned_pid = waitpid(game->pid, &game->status, options); // WTRAPPED and WEXITED are set implicitly for waitpid, but meh
    __rerrv(returned_pid == -1, -1, "waipid returned error: %s", strerror(errno));
    
    
    if (returned_pid == 0)
    {
        __rerrv(wait != 0, -1, "NOHANG was not specified, but waitpid returned 0 anyway.");

#if VERBOSITY > 1
        echo("No processes wishing to report status.");
#endif
        return returned_pid;
    }
    

#if VERBOSITY > 1
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

        if (WSTOPSIG(game->status) == 11)
        {
            echo("SEGMENTATION FAULT");
        }
        else{
            echo("Stopped (sig: %d)", WSTOPSIG(game->status));
        }
    }
#endif





    // Get thread info
    int ret;
    errno = 0;
    struct ptrace_lwpinfo thread_info;
    ret = ptrace(PT_LWPINFO, game->pid, (caddr_t) &thread_info, sizeof(struct ptrace_lwpinfo));
    __rerrv(ret != 0, -1, "Unable to get lwp info for stopped process in pid %d", game->pid)

#if VERBOSITY > 1
    echo("Thread that stopped / terminated: %s (%d:%d)", thread_info.pl_tdname, returned_pid, thread_info.pl_child_pid);
#endif
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

#if VERBOSITY > 1
    echo("Resuming all threads in process %d", pid);
#endif

    
    // Get number of threads1
    errno = 0;
    thread_count = ptrace(PT_GETNUMLWPS, pid, 0, 0);
    __rerrv(thread_count <  0, 1, "Error getting thread id list. PT_GETNUMLWPS returned a negative value.");
    __rerrv(thread_count == 0, 1, "Error getting thread id list. Process contains no threads??");




    // Allocate space for the thread ids
    threads = (lwpid_t *) malloc(sizeof(lwpid_t) * thread_count);
    __err(threads == 0, "Unable to allocate the %d bytes needed for getting thread ids.", (sizeof(lwpid_t) * thread_count));


    // Get threads themselves
    errno = 0;
    ret = ptrace(PT_GETLWPLIST, pid, (caddr_t) threads, thread_count);
    __rerrv(ret != thread_count, 1, "Error getting thread id list. Only read %d out of %d threads.", ret, thread_count);
    __rerrv(!ret, 1, "Error getting thread id list. (PT_GETLWPLIST returned error)");




    errno = 0;
    for(int i = 0; i < thread_count; i++)
    {
        ret = ptrace(PT_RESUME, threads[i], 0, 0);
        __errg(ret, _break, "Error resuming thread #%d out of %d", i, thread_count);
    }
_break:

    free(threads);
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
    __rerrv(thread_count < 0, 1, "Error getting thread id list. PT_GETNUMLWPS returned a negative value.");
    __rerrv(thread_count == 0, 1, "Error getting thread id list. Process contains no threads??");


    // Allocate space for the thread ids
    threads = (lwpid_t *) malloc(sizeof(lwpid_t) * thread_count);
    if (!threads)
    {
        echo("ERROR: unable to allocate %d bytes to get thread ids.");
    }


    // Get threads themselves
    ret = ptrace(PT_GETLWPLIST, pid, (caddr_t) threads, thread_count);
    __errg(ret != thread_count, _ret, "Error getting thread id list. Only read %d out of %d threads.", ret, thread_count);
    __errg(!ret, _ret, "Error getting thread id list. (PT_GETLWPLIST returned error)");


    
    for(int i = 0; i < thread_count; i++)
    {
        ret = ptrace(PT_SUSPEND, threads[i], 0, 0);
        if (errno != 0)
        {
            perror("PT_SUSPEND: ");
            echo("Error suspending thread #%d out of %d", i, thread_count);
            free(threads);
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

_ret:
    free(threads);
    return ret != 0;
}








int proc_continue()
{
    #if VERBOSITY > 1
    echo("Continuing %d", game->pid);
    #endif

    int ret;
    ret = ptrace(PT_CONTINUE, game->pid, (caddr_t) 1, SIGCONT);
    if (ret)
    {
        echo("ERROR:\n# PT_CONTINUE failed with code %d (%s)", ret, strerror(ret));
        __err(1, (char *) nullptr);
    }

    // ret = proc_resume_threads(game->pid);
    // __err(ret != 0, "proc_resume_threads failed (pid: %d)", game->pid));
    return ret != 0;
}




int proc_stop()
{
    #if VERBOSITY > 1
    echo("Stopping %d", game->pid);
    #endif

    int ret;
    ret = kill(game->pid, SIGSTOP);
    __err(ret != 0, "kill(%d, SIGSTOP) failed", game->pid);

    // ret = proc_suspend_threads(game->pid);
    // __err(ret != 0, "proc_suspend_threads failed (pid: %d)", game->pid));
    return ret != 0;
}







int proc_get_regs(struct reg * regs)
{
    int ret;
    ret = ptrace(PT_GETREGS, game->pid, (caddr_t) regs, 0);

    __rerrv(ret != 0, 1, "PT_GETREGS failed with code %d (%s)", ret, strerror(ret));

    #if VERBOSITY > 2
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
    int ret = ptrace(PT_SETREGS, game->pid, (caddr_t) regs, 0);
    __rerrv(ret != 0, 1, "PT_SETREGS failed with code %d (%s)\n", ret, strerror(ret));
    
    
    #if VERBOSITY > 2
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
    int ret = ptrace(PT_GETDBREGS, game->pid, (caddr_t) dbregs, 0);
    __rerrv(ret != 0, 1, "PT_GETDBREGS failed with code %d (%s)\n", ret, strerror(ret));

    return 0;
}


int proc_set_dbregs(struct dbreg * dbregs)
{
    int ret = ptrace(PT_SETDBREGS, game->pid, (caddr_t) dbregs, 0);
    __rerrv(ret != 0, 1, "PT_SETDBREGS failed with code %d (%s)\n", ret, strerror(ret));
    
    return 0;
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
    pid_t pid = game->pid;
    thread_count = ptrace(PT_GETNUMLWPS, pid, null, 0);
    __rerrv(thread_count == -1, 1, "PT_GETNUMLWPS failed. Aborting watchpoint creation")
    __rerrv(thread_count < 1, 1, "PT_GETNUMLWPS returned invalid number of threads. Aborting watchpoint creation. (%d)", thread_count)



    // Get the threads themselves
    lwpid_list = (lwpid_t *) malloc(thread_count * sizeof(lwpid_t));
    ret = ptrace(PT_GETLWPLIST, pid, (caddr_t) lwpid_list, thread_count);
    __errg(ret != thread_count, _retErr, "PT_GETLWPLIST failed. Did not fill all array entries. (%d != %d)", ret, thread_count)
    
    
    

    // Get the current debug register values to avoid overwriting other watchpoints
    ret = ptrace(PT_GETDBREGS, pid, (caddr_t) &regs, 0);
    __errg(ret != 0, _retErr, "PT_GETDBREGS failed with code %d (%s)", ret, strerror(ret))


    
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
            ret = ptrace(PT_LWPINFO, lwpid_list[i], (caddr_t) &lwpinfo, sizeof(struct ptrace_lwpinfo));
            __errg(ret != 0, _retErr, "PT_LWPINFO failed with code (%s) %d", strerror(ret), ret)

            echo(lwpinfo.pl_tdname);
        }


        ret = ptrace(PT_SETDBREGS, lwpid_list[i], (caddr_t) &regs, 0);
        __errg(ret != 0, _retErr, "PT_SETDBREGS failed with code (%s) %d", strerror(ret), ret)
    }
    
    
    free(lwpid_list);
    return 0;

_retErr:
    free(lwpid_list);
    return 1;
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
    __rerrv(regs == 0, 1, "Null regs pointer provided.");
    int ret;
    

    // Get the current register values to ensure critical registers like RSP are still filled
    __err((ret = proc_get_regs(regs)), "proc_get_regs failed, unable to get register values when clearing args. Clearing general data registers anyway, but returning error.")


    // Apply register values
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

    
    //! I guess we can't actually read from that address? Can we only read after the header?
    // int ret;
    // ret = read_byte((void *) 0x400000);
    // __rerrv(ret != 0xCC, errmsg, 1);
    

    // Back the regisers up
    int ret;
    struct reg og_registers;
    __rerrv(proc_get_regs(&og_registers), 1, "Failed getting original register values before function call in tracee.");

    
    
    // Default to current register values
    if (!arg_regs)
    {
        echo("No args provided, using og_registers");
        arg_regs = &og_registers;
    }
    
    
    

    // Set return address
    ret = write_ulong((void *) (arg_regs->r_rsp -= 8), (unsigned long) 0x400000);
    __rerrv(ret != 0, 1, "Failed setting return address, aborting function call");
    
    // Set next instruction address
    arg_regs->r_rip = (critical_t) function_address;
    
    __rerrv(proc_set_regs(arg_regs), 1, "Failed setting new register values, aborting function call.");

    
    
    
    // Resume execution until the function returns, hitting the breakpoint present at the start of every ps5 elf
    struct reg return_regs;
    __errg(proc_continue(), _return, "proc_continue returned failure status, subsequent errors likely.");

    __errg(proc_wait(true) == -1, _return, "proc_wait returned error status when wiating for setup function call to return, subsequent errors likely.");

    __errg(proc_get_regs(&return_regs), _return, "Failed to read new register values after function returned/stopped");

    if (return_regs.r_rip != 0x400001)
    {
        //! TODO: Get additional information about the stop (at least the signal that caused it)
        echo("# Registers (READ):");
        echo("# rax: %llX  rbx: %llX", return_regs.r_rax, return_regs.r_rbx);
        echo("# rcx: %llX  rdx: %llX", return_regs.r_rcx, return_regs.r_rdx);
        echo("# rsi: %llX  rdi: %llX", return_regs.r_rsi, return_regs.r_rdi);
        echo("# rbp: %llX  rsp: %llX", return_regs.r_rbp, return_regs.r_rsp);
        echo("# r8:  %llX  r9:  %llX", return_regs.r_r8,  return_regs.r_r9);
        echo("# r10: %llX  r11: %llX", return_regs.r_r10, return_regs.r_r11);
        echo("# r12: %llX  r13: %llX", return_regs.r_r12, return_regs.r_r13);
        echo("# r14: %llX  r15: %llX", return_regs.r_r14, return_regs.r_r15);
        echo("# rip: %llX  eflags: %llX", return_regs.r_rip, return_regs.r_rflags);
        echo("# trapno:  %llX  err: %d", return_regs.r_trapno, return_regs.r_err);
        echo("");
        
        __errg(return_regs.r_rip != 0x400001, _return, "Process stopped before returning from call to 0x%llX. (Next instruction when stopped: 0x%llX)", function_address, return_regs.r_rip);
    }

    if (return_val_ptr > 0)
    {
        // Read return value
        *(long long *) return_val_ptr = return_regs.r_rax;
#if VERBOSITY > 1
        echo("Function at 0x%llX returned: 0x%llX", function_address, *return_val_ptr);
#endif
    }
#if VERBOSITY > 1
    else {
        echo("Function at 0x%llX returned.", function_address);
    }
#endif
    



_return:
    // Restore the initial register values
    ret = proc_set_regs(&og_registers);
    __rerrv(ret, 1, "Failed to restore original register values after function call, subsequent errors likely");
    
    return 0;
}








void * proc_malloc(size_t amount)
{
    void * mem;      // Allocated memory pointer
    struct reg regs; // Register struct for call args


    // set malloc arguments
    __rerrv(proc_set_args(&regs, amount, 0, 0, 0, 0, 0, 0), nullptr, "Failed to set args for proceeding function call to tracee's malloc function (0x%llX).", game->Addrs.Malloc)
    
    #if VERBOSITY > 1
    echo("Allocating 0x%lX bytes in tracee", amount);
    #endif
    
    // call malloc
    __rerrv(proc_call(game->Addrs.Malloc, (unsigned long *) &mem, &regs), nullptr, "Call to tracee's malloc function (0x%llX) failed", game->Addrs.Malloc)

    return mem;
}




int proc_free(void * allocated_mem)
{
    struct reg regs; // Register struct for call args

    // set free arguments
    __rerrv(proc_set_args(&regs, (uintptr_t) allocated_mem, 0, 0, 0, 0, 0, 0), 1, "Failed to set args for proceeding function call to tracee's free function (0x%llX).", game->Addrs.Free);

    #if VERBOSITY > 1
    echo("Freeing allocated memory in tracee at 0x%llX", allocated_mem);
    #endif
    
    // Call free
    __rerrv(proc_call((void *) game->Addrs.Free, NULL, &regs), 1, "Call to tracee's free function (0x%llX) failed", game->Addrs.Free)
    
    return 0;
}

