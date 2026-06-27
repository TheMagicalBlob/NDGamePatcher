#include "inc/include.h"
#include "inc/memory.h"
#include "inc/dmenu.h"


#define printf(s, ...) klog_printf(s, __VA_ARGS__)





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
//## ptrace functions
//#



int attach(int pid)
{
    int status;
    notify("Attaching to process...");

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
    if ((ret = ptrace(PT_GETREGS, pid, (caddr_t) &regs, 0)) != 0)
    {
        printf("ERROR:\n# PT_GETREGS failed with code %d (%s)\n", ret, strerror(ret));
        return -1;
    }
}


int set_regs(pid_t pid, struct reg * regs)
{
    int ret;
    if ((ret = ptrace(PT_SETREGS, pid, (caddr_t) &regs, 0)) != 0)
    {
        printf("ERROR:\n# PT_SETREGS failed with code %d (%s)\n", ret, strerror(ret));
        return -1;
    }
}



int proc_call(pid_t pid, void * function_address, void * return_val_ptr, struct reg * arg_regs)
{
    if (function_address == 0)
    {
        echo("ERROR: Null address provided for function.");
        return 1;
    }



    // Get a copy of the initial register values
    int ret;
    struct reg registers;
    if ((ret = ptrace(PT_GETREGS, pid, (caddr_t) &registers, 0)) != 0)
    {
        printf("ERROR:\n# PT_GETREGS failed with code %d (%s)\n", ret, strerror(ret));
        return -1;
    }

    
    // Back the regisers up
    struct reg og_registers;
    memcpy(&og_registers, &registers, sizeof(struct reg));
    
    
    
    // Set return address
    long long og_stack_ptr  = (long long) read_ulong(pid, (unsigned long *) registers.r_rsp);

    write_ulong(pid, (void *) registers.r_rsp - 8, (unsigned long) registers.r_rip);
    registers.r_rip = (long long) function_address;
    
    
    if ((ret = ptrace(PT_SETREGS, pid, (caddr_t) &registers, 0)) != 0)
    {
        printf("ERROR:\n# PT_SETREGS failed with code %d (%s)\n", ret, strerror(ret));
        return -1;
    }


    struct reg backup_reg;

    while (1)
    {
        ret = ptrace(PT_STEP, pid, 0, 0);
        if (ret != 0){
            printf("ERROR:\n# PT_STEP failed with code %d (%s) while checking rip.\n", ret, strerror(ret));
            return -1;
        }
            
        waitpid(pid, 0, 0);
        if ((ret = ptrace(PT_GETREGS, pid, (caddr_t) &registers, 0)) != 0)
        {
            printf("ERROR:\n# PT_GETREGS failed with code %d (%s) while checking rip.\n", ret, strerror(ret));
            return -1;
        }


        memcpy(&backup_reg, &registers, sizeof(registers));


        if (registers.r_rip == (__int64_t) og_stack_ptr)
        {
            if (return_val_ptr > 0)
            {
                printf("Return address reached, getting return value (%llx)\n", registers.r_rax);
                *((long long *) return_val_ptr) = registers.r_rax;
            }

            break;
        }
    }


    
    // Restore the registers 
    if ((ret = ptrace(PT_SETREGS, pid, (caddr_t) &og_registers, 0)) != 0)
    {
        printf("ERROR:\n# PT_SETREGS failed with code %d (%s)\n", ret, strerror(ret));
        return -1;
    }


    return 0;
}




void * proc_malloc(pid_t pid, int amount)
{
    int ret;
    void * ptr; 

    struct reg regs;
    if ((ret = ptrace(PT_GETREGS, pid, (caddr_t) &regs, 0)) != 0)
    {

    }

    if ((ret = proc_call(pid, (void *) T2_MALLOC, &ptr, NULL)) != 0)
    {
        echo("ERROR: malloc call failed! (ret: %d, amount: %d)", ret, amount);
        return nullptr;
    }

    echo("allocate mem ptr: %llX", ptr);

    return ptr;
}


int proc_free(void * mem_ptr)
{
    return -1; //!
}




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

static int proc_read(pid_t pid, int len, void * addr, void * buffer, const char * caller, int from)
{
    struct ptrace_io_desc iod = { PIOD_READ_D, addr, buffer, len };

    int ret;
    if ((ret = ptrace(PT_IO, pid, (caddr_t) &iod, 0)) != 0)
    {
        printf("ERROR:\n# ptrace PT_IO (read) failed with code %d (%s)\n\n# Address: %llX\n# Function: %s\n# Line: %d\n", ret, strerror(ret), addr, caller, from);
        return 1;
    }

    return 0;
}




static int proc_write(pid_t pid, int len, void * addr, void * value, const char * caller, int from)
{
    struct ptrace_io_desc iod = { PIOD_WRITE_D, addr, value, len };

    int ret;
    if ((ret = ptrace(PT_IO, pid, (caddr_t) &iod, 0)) != 0)
    {
        printf("ERROR:\n# ptrace PT_IO (read) failed with code %d (%s)\n\n# Address: %llX\n# Function: %s\n# Line: %d\n", ret, strerror(ret), addr, caller, from);
        return 1;
    }

    return 0;
}




/// Read length number of bytes in the specified process in to buffer, starting at address.
int read_bytes(int pid, size_t length, void * address, void * buffer)
{
    return proc_read(pid, length, address, buffer, __FUNCTION__, __LINE__);
}




int read_string(int pid, void * address, void * buffer)
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




unsigned long long read_ulong(int pid, unsigned long * address)
{
    unsigned long err = 0xDEADBEEF;
    unsigned long long data = err;

    if (proc_read(pid, 8, address, &data, __FUNCTION__, __LINE__))
    {
        return err;
    }

    return data;
}





long double read_double(int pid, unsigned long address)
{
    int ret;
    double data;
    struct ptrace_io_desc iod = { 1, (void *) address, &data, 0x8 };

    if ((ret = ptrace(PT_IO, pid, (caddr_t) &iod, 0)) != 0)
    {
        echo("ERROR:\n# ptrace PT_IO failed with code: %s\n\n# Function: %s\n# Line: %d", strerror(ret), __FUNCTION__, __LINE__);
        return 0xDEADBEEF;
    }

    return data;
}





unsigned int read_uint(int pid, void * address)
{
    int ret;
    unsigned int data = 0xDEADBEEF;
    struct ptrace_io_desc iod = { 1, address, &data, 0x4 };

    if ((ret = ptrace(PT_IO, pid, (caddr_t) &iod, 0)) != 0)
    {
        echo("ERROR:\n# ptrace PT_IO failed with code: %s\n\n# Function: %s\n# Line: %d", strerror(ret), __FUNCTION__, __LINE__);
        return 0xDEADBEEF;
    }

    return data;
}





float read_single(int pid, unsigned long address)
{
    int ret;
    float data;
    struct ptrace_io_desc iod = { 1, (void *) address, &data, 0x4 };

    if ((ret = ptrace(PT_IO, pid, (caddr_t) &iod, 0)) != 0)
    {
        echo("ERROR:\n# ptrace PT_IO failed with code: %s\n\n# Function: %s\n# Line: %d", strerror(ret), __FUNCTION__, __LINE__);
        return (float) 0xDEADBEEF;
    }

    return data;
}





unsigned char read_byte(int pid, void * address)
{
    int ret;
    unsigned char data;
    struct ptrace_io_desc iod = { 1, address, &data, 0x1 };

    if ((ret = ptrace(PT_IO, pid, (caddr_t) &iod, 0)) != 0)
    {
        echo("ERROR:\n# ptrace PT_IO failed with code: %s\n\n# Function: %s\n# Line: %d", strerror(ret), __FUNCTION__, __LINE__);
        return 0;
    }

    return data;
}





int write_ulong(int pid, unsigned long * address, unsigned long value)
{
    unsigned long long data = value;

    if (proc_write(pid, 8, address, &data, __FUNCTION__, __LINE__))
    {
        return 1;
    }

    return 0;
}