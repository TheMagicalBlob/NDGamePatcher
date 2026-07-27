#include "inc/include.h"
#include "inc/dmenu.h"

#define printf(s, ...) klog_echo(s, __VA_ARGS__)

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





int set_dbg_perms()
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





//#
//## ptrace debugging functions
//#

int attach(nd_game_t nd_game)
{
    int ret;
    if ((ret = ptrace(PT_ATTACH, nd_game.pid, 0, 0)) != 0)
    {
        notify("PT_ATTACH ERROR: %s\n", strerror(ret));
        
        if(kill(nd_game.pid, SIGCONT))
        {
            echo("WARNING: Unable to resume process through SIGCONT");
        }
    }
    
    echo("Attached to pid %d", nd_game.pid);
    return ret;
}


int detach(nd_game_t * nd_game, const char * from)
{
    if (nd_game->pid <= 0)
    {
        echo("%s: Invalid pid provided (%d).", __FUNCTION__, nd_game->pid);
        return 1;
    }
    echo("Detaching from process %d (%s)", nd_game->pid, from);


    int status;
    if ((status = ptrace(PT_DETACH, nd_game->pid, 0, 0)) != 0)
    {
        perror("PT_DETACH ERROR: ");
    }
    else{
        notify("Detached from %d, exiting.", nd_game->pid);
    }

    echo("Detached from process, sending sigcont and zeroing nd_game struct");
    kill(nd_game->pid, SIGCONT);
    bzero(nd_game, sizeof(nd_game_t));

    return status;
}






// Wait for the secified process to either be trapped, or exit
// Returns the result of WIFEXITED
int proc_wait(nd_game_t * game)
{
    echo("waiting for pid %d.", game->pid);

    if (waitpid(game->pid, &game->status, WTRAPPED | WEXITED/*| WUNTRACED*/) == -1) // WTRAPPED and WEXITED are set implicitly for waitpid, but I'm leaving them to remind me what the args are without re-reading the docs if I forget
    {
        echo("waitpid: no children not previously awaited");
    }

    echo("Status of stopped process: %lX", game->status);


    return WIFEXITED(game->status);
}






int proc_suspend(pid_t pid)
{
    // fairly sure I'd need to use PT_SUSPEND on every thread, so screw it, I'll just use the stop signal
    if (kill(pid, SIGSTOP) != 0)
    {
        echo("Error suspending process; whatever we're doing is about to crash this payload and kill the game process");
        return 1;
    }

    return 0;
}






int proc_continue(pid_t pid, void * resume)
{
    int ret;
    if (resume == nullptr)
    {
        resume = (void *) 1; // (continue where it left off)
    }

    if ((ret = ptrace(PT_CONTINUE, pid, resume, 0)) != 0)
    {
        echo("ERROR:\n# PT_CONTINUE failed with code %d (%s)\n", ret, strerror(ret));
    }

    return ret;
}






int get_regs(pid_t pid, struct reg * regs)
{
    int ret;
    if ((ret = ptrace(PT_GETREGS, pid, (caddr_t) regs, 0)) != 0)
    {
        echo("ERROR:\n# PT_GETREGS failed with code %d (%s)\n", ret, strerror(ret));
    }

    return ret;
}


int set_regs(pid_t pid, struct reg * regs)
{
    int ret;
    if ((ret = ptrace(PT_SETREGS, pid, (caddr_t) regs, 0)) != 0)
    {
        echo("ERROR:\n# PT_SETREGS failed with code %d (%s)\n", ret, strerror(ret));
    }
    
    return ret;
}






static struct dbreg regs;
static struct ptrace_lwpinfo lwpinfo;
int set_watchpoint(pid_t pid, short index, void * address, int wp_type, int wp_len)
{
    int ret;
    int thread_count;
    lwpid_t * lwpid_list;
    
    if (index > 3 || index < 0)
    {
        echo("ERROR: Invalid index supplied for watchpoint creation.");
        return 1;
    }
    
    // # Get the amount of threads, and the threads themselves
    if ((thread_count = ptrace(PT_GETNUMLWPS, pid, null, 0)) == -1)
    {
        perror("PT_GETNUMLWPS: ");
        echo("ERROR:\n# PT_GETNUMLWPS failed with code %d (%s)", thread_count, strerror(thread_count));
        return 1;
    }
    

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
    // regs.dr[7] |= (3 << 8); // exact breakpoint enable for LE & GE
    regs.dr[7] = 0x0000000000000700; // exact breakpoint enable for LE & GE
    
    if (address)
    {
        regs.dr[7] |= DBREG_DR7_SET(index, wp_type ? wp_len : DBREG_DR7_LEN_1, wp_type, DBREG_DR7_LOCAL_ENABLE | DBREG_DR7_GLOBAL_ENABLE);  // len=4 => DBREG_DR7_LEN_4 (macro encodes bits)
    }
    else {
        regs.dr[7] |= DBREG_DR7_SET(index, 0, 0, 0);
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

    
    
    if (index < 0)
    {
        echo("Clearing all watchpoints");

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
    
    echo("Clearing watchpoint #%d", index);
    return set_watchpoint(pid, index, NULL, 0, 0);
}






unsigned long long read_ulong(int pid, unsigned long * address);
int write_ulong(int pid, unsigned long * address, unsigned long value);
// Setup a function call at the specified function_address in the tracee.
int proc_call(pid_t pid, void * function_address, unsigned long * return_val_ptr, struct reg * arg_regs)
{
    if (function_address == 0)
    {
        echo("ERROR: Null address provided for function.");
        return 1;
    }






    u_long error_source;



    // Back the regisers up
    echo("getting og regisers");
    struct reg og_registers;
    if (get_regs(pid, &og_registers))
    {
        error_source = __LINE__ - 2;
        goto _err;
    }


    
    // Make a copy of the current register values if no existing struct ref was provided
    struct reg registers;
    if (arg_regs == nullptr)
    {
        echo("no args provided, copying og regisers to use instead");
        memcpy(&registers, &og_registers, sizeof(struct reg));
        arg_regs = &registers;
    }
    
    
    

    // Set up the function call
    unsigned long og_next_instruction  = (long long) arg_regs->r_rip; // save return stack pointer

    write_ulong(pid, (void *) arg_regs->r_rsp - 8, (unsigned long) arg_regs->r_rip); // set return address

    arg_regs->r_rip = (long long) function_address; // Set next instruction address
    
    if (set_regs(pid, arg_regs))
    {
        error_source = __LINE__ - 2;
        goto _err;
    }





    int ret;
    ret = set_watchpoint(pid, 0, (void *) og_next_instruction, DBREG_DR7_EXEC, DBREG_DR7_LEN_1);
    if (ret)
    {
        echo("error setting watchpoint");
        error_source = __LINE__;
        goto _err;
    }

    proc_continue(pid, null);

    nd_game_t * nd_game;
    nd_game = (nd_game_t *) get_nd_game();

    proc_wait(nd_game);


    // Get return value




    clear_watchpoint(pid, 0);


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
    if (get_regs(pid, &og_registers))
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
    
    if (set_regs(pid, arg_regs))
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
    


        if (get_regs(pid, arg_regs))
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
            if (set_regs(pid, &og_registers))
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




void * proc_malloc(nd_game_t * game, int amount)
{
    int ret;
    void * allocated_mem; 

    struct reg regs;
    if (get_regs(game->pid, &regs))
    {
        return nullptr;
    }


    regs.r_rdi = amount;


    ret = proc_call(game->pid, (void *) game->func_malloc, (unsigned long *) &allocated_mem, &regs);
    if (ret)
    {
        echo("ERROR: malloc call failed! (ret: %d, amount: %d)", ret, amount);
        return nullptr;
    }

    echo("allocate mem ptr: %llX", allocated_mem);

    return allocated_mem;
}


int proc_free(nd_game_t * game, void * allocated_mem)
{
    int ret;
    struct reg regs;

    regs.r_rdi = (long long) allocated_mem;

    if ((ret = get_regs(game->pid, &regs)))
    {
        echo("ERROR: free get_regs call failed!");
    }
    else if ((ret = proc_call(game->pid, (void *) game->func_free, NULL, &regs)))
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

static int proc_read(pid_t pid, int len, void * addr, void * buffer, const char * caller)
{
    if (!addr || !buffer)
    {
        echo("ERROR:\n# %s->ptrace PT_IO (read)\nInput or output address was null (addr: %llX  buffer: %llX)", caller, addr, buffer);
        return 1;
    }


    struct ptrace_io_desc iod = { PIOD_READ_D, addr, buffer, len };

    int ret;
    if ((ret = ptrace(PT_IO, pid, (caddr_t) &iod, 0)) != 0)
    {
        echo("ERROR:\n# ptrace PT_IO (read) failed with code %d (%s)\n\n# Address: %llX\n# Function: %s\n", ret, strerror(ret), addr, caller);
        return 1;
    }

    return 0;
}




static int proc_write(pid_t pid, int len, void * addr, void * value, const char * caller)
{
    if (!addr || !value)
    {
        echo("ERROR:\n# ptrace PT_IO (write)\nInput or output address was null");
        return 1;
    }
    
    
    struct ptrace_io_desc iod = { PIOD_WRITE_D, addr, value, len };

    int ret;
    if ((ret = ptrace(PT_IO, pid, (caddr_t) &iod, 0)) != 0)
    {
        echo("ERROR:\n# ptrace PT_IO (read) failed with code %d (%s)\n\n# Address: %llX\n# Function: %s\n", ret, strerror(ret), addr, caller);
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

    if ((ret = proc_read(pid, 1, address, &data, __FUNCTION__)) != 0)
    {
        return -1;
    }

    return data;
}


int write_byte(int pid, void * address, unsigned char data)
{
    return proc_write(pid, 1, address, &data, __FUNCTION__);
}





unsigned long long read_ulong(int pid, unsigned long * address)
{
    unsigned long long data;

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

