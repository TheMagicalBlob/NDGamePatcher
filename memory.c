#include "inc/include.h"
#include "inc/dmenu.h"

#define printf(s, ...) klog_printf(s, __VA_ARGS__)

#define _SHELLCORE_AUTHID 0x4800000000010003L





// from john-tornblom
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
        if (!strcmp(name, ki_tdname) && ki_pid != mypid) {
            pid = ki_pid;
        }
    }

    free(buf);

    return pid;
}





int set_dbg_perms()
{
    echo("Setting authid...");
    
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



int attach(int pid)
{
    echo("Attaching to process...");
    
    int status;
    if ((status = ptrace(PT_ATTACH, pid, 0, 0)) != 0)
    {
        notify("PT_ATTACH ERROR: %s\n", strerror(status));
        
        if(kill(pid, SIGCONT))
        {
            echo("WARNING: Unable to resume process through SIGCONT");
        }
    }

    return status;
}





int detach(int pid)
{
    echo("Detaching from process %d", pid);

    int status;
    if ((status = ptrace(PT_DETACH, pid, 0, 0)) != 0)
    {
        char err[0x200];
        snprintf(err, sizeof(err), "PT_DETACH ERROR: %s\n\nProcess may remain paused; suggest kill(pid, SIGCONT)", strerror(status));
        
        notify(err);
    }
    else{
        notify("Detached, exiting.");
    }

    return status;
}




int get_regs(pid_t pid, struct reg * regs)
{
    int ret;
    if ((ret = ptrace(PT_GETREGS, pid, (caddr_t) regs, 0)) != 0)
    {
        printf("ERROR:\n# PT_GETREGS failed with code %d (%s)\n", ret, strerror(ret));
    }

    return ret;
}


int set_regs(pid_t pid, struct reg * regs)
{
    int ret;
    if ((ret = ptrace(PT_SETREGS, pid, (caddr_t) regs, 0)) != 0)
    {
        printf("ERROR:\n# PT_SETREGS failed with code %d (%s)\n", ret, strerror(ret));
    }

    return ret;
}




unsigned long long read_ulong(int pid, unsigned long * address);
int write_ulong(int pid, unsigned long * address, unsigned long value);



// Setup a function call at the specified function_address in the tracee process.
// Opt
int proc_call(pid_t pid, void * function_address, unsigned long * return_val_ptr, struct reg * arg_regs)
{
    if (function_address == 0)
    {
        echo("ERROR: Null address provided for function.");
        return 1;
    }
    
    int ret;
    u_long error_source;

    if (0)
    {
    err:
        echo("Unable to complete function call (failed at line %d)", error_source);
        return 1;
    }





    // Back the regisers up
    echo("getting og regisers");
    struct reg og_registers;
    if (get_regs(pid, &og_registers))
    {
        error_source = __LINE__ - 2;
        goto err;
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
    long long og_stack_ptr  = (long long) read_ulong(pid, (unsigned long *) arg_regs->r_rsp); // save return stack pointer

    write_ulong(pid, (void *) arg_regs->r_rsp - 8, (unsigned long) arg_regs->r_rip); // set return address

    arg_regs->r_rip = (long long) function_address; // Set next instruction address
    
    if (set_regs(pid, arg_regs))
    {
        error_source = __LINE__ - 2;
        goto err;
    }



    //! TODO: replace this, fairly sure I can just use a breakpoint
    // Step through until we return from the called function, then restore the original register values (and read the return value if a pointer to store it at was provided)
    while (1)
    {
        if ((ret = ptrace(PT_STEP, pid, 0, 0)) != 0)
        {
            printf("ERROR:\n# PT_STEP failed with code %d (%s) while checking rip.\n", ret, strerror(ret));
            return 1;
        }
        waitpid(pid, 0, 0);
    


        if (get_regs(pid, arg_regs))
        {
            error_source = __LINE__ - 2;
            goto err;
        }


        if (arg_regs->r_rip == (__int64_t) og_stack_ptr)
        {
            if (return_val_ptr > 0)
            {
                *((long long *) return_val_ptr) = arg_regs->r_rax;
                echo("Getting return value %llX", arg_regs->r_rax);
            }
            
            // Restore the registers 
            if (set_regs(pid, &og_registers))
            {
                error_source = __LINE__ - 2;
                goto err;
            }

            break;
        }
    }


    return 0;
}




void * proc_malloc(pid_t pid, int amount)
{
    int ret;
    void * allocated_mem; 

    struct reg regs;
    if (get_regs(pid, &regs))
    {
        return nullptr;
    }


    regs.r_rdi = amount;


    ret = proc_call(pid, (void *) T2_MALLOC, (unsigned long *) &allocated_mem, &regs);
    if (ret)
    {
        echo("ERROR: malloc call failed! (ret: %d, amount: %d)", ret, amount);
        return nullptr;
    }

    echo("allocate mem ptr: %llX", allocated_mem);

    return allocated_mem;
}


int proc_free(pid_t pid, void * allocated_mem)
{
    int ret;
    struct reg regs;

    regs.r_rdi = (long long) allocated_mem;

    if ((ret = get_regs(pid, &regs)))
    {
        echo("ERROR: free get_regs call failed!");
    }
    else if ((ret = proc_call(pid, (void *) T2_FREE, NULL, &regs)))
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
    struct ptrace_io_desc iod = { PIOD_READ_D, addr, buffer, len };

    int ret;
    if ((ret = ptrace(PT_IO, pid, (caddr_t) &iod, 0)) != 0)
    {
        printf("ERROR:\n# ptrace PT_IO (read) failed with code %d (%s)\n\n# Address: %llX\n# Function: %s\n", ret, strerror(ret), addr, caller);
        return 1;
    }

    return 0;
}




static int proc_write(pid_t pid, int len, void * addr, void * value, const char * caller)
{
    struct ptrace_io_desc iod = { PIOD_WRITE_D, addr, value, len };

    int ret;
    if ((ret = ptrace(PT_IO, pid, (caddr_t) &iod, 0)) != 0)
    {
        printf("ERROR:\n# ptrace PT_IO (read) failed with code %d (%s)\n\n# Address: %llX\n# Function: %s\n", ret, strerror(ret), addr, caller);
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
// Returns the amount of characters written, or -1 if a null pointer was provided for either the source or the destination
int read_string(int pid, void * address, char * buffer)
{
    if (address == 0)
    {
        echo("null string pointer provided.");
        return -1;
    }

    if (read_byte(pid, address) <= 0) 
    {
        echo("Empty string at address 0x%llX.", (unsigned long long) address);
        return 0;
    }
    
 
    int len = 0;
    for (; read_byte(pid, address + len) != 0; ++len);
    
    
    
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
    while (*(str + ++len) != 0);
    
    for(long long i = 0; i < len; ++i)
    {
        write_byte(pid, (void *) address + i, *(char *) (str + i));
    }
    *(char *) (str + len) = 0; // terminate the string

    return len;
}

