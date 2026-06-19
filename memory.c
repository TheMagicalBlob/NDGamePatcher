#include "inc/include.h"
#include "inc/memory.h"


int echo(char * str);

void notify(const char * message, ...);





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
    printf("Detaching from process %d\n", pid);

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
    struct ptrace_io_desc iod = { 1, addr, buffer, len };

    int ret;
    if ((ret = ptrace(PT_IO, pid, (caddr_t) &iod, 0)) != 0)
    {
        printf("ERROR:\n# ptrace PT_IO failed with code %d\n# Message: %s\n\n# Function: %s\n# Line: %d", ret, strerror(ret), caller, from);
        return 1;
    }

    return 0;
}




/// Read length number of bytes in the specified process in to buffer, starting at address.
int read_bytes(int pid, size_t length, void * address, void * buffer)
{
    return proc_read(pid, length, address, buffer, __FUNCTION__, __LINE__);
}





unsigned long read_ulong(int pid, unsigned long address)
{
    unsigned long err = 0xDEADBEEF;
    unsigned long data = err;

    if (proc_read(pid, 8, (void *) address, &data, __FUNCTION__, __LINE__))
    {
        return err;
    }

    return data;
}





unsigned int read_double(int pid, unsigned long address)
{
    int ret;
    double data;
    struct ptrace_io_desc iod = { 1, (void *) address, &data, 0x8 };

    if ((ret = ptrace(PT_IO, pid, (caddr_t) &iod, 0)) != 0)
    {
        printf("ERROR:\n# ptrace PT_IO failed with code: %s\n\n# Function: %s\n# Line: %d", strerror(ret), __FUNCTION__, __LINE__);
        return 0xDEADBEEF;
    }

    return data;
}





unsigned int read_uint(int pid, unsigned long address)
{
    int ret;
    unsigned int data = 0xDEADBEEF;
    struct ptrace_io_desc iod = { 1, (void *) address, &data, 0x4 };

    if ((ret = ptrace(PT_IO, pid, (caddr_t) &iod, 0)) != 0)
    {
        printf("ERROR:\n# ptrace PT_IO failed with code: %s\n\n# Function: %s\n# Line: %d", strerror(ret), __FUNCTION__, __LINE__);
        return 0xDEADBEEF;
    }

    return data;
}





unsigned int read_single(int pid, unsigned long address)
{
    int ret;
    float data;
    struct ptrace_io_desc iod = { 1, (void *) address, &data, 0x4 };

    if ((ret = ptrace(PT_IO, pid, (caddr_t) &iod, 0)) != 0)
    {
        printf("ERROR:\n# ptrace PT_IO failed with code: %s\n\n# Function: %s\n# Line: %d", strerror(ret), __FUNCTION__, __LINE__);
        return 0xDEADBEEF;
    }

    return data;
}





unsigned int read_byte(int pid, unsigned long address)
{
    int ret;
    unsigned char data;
    struct ptrace_io_desc iod = { 1, (void *) address, &data, 0x1 };

    if ((ret = ptrace(PT_IO, pid, (caddr_t) &iod, 0)) != 0)
    {
        printf("ERROR:\n# ptrace PT_IO failed with code: %s\n\n# Function: %s\n# Line: %d", strerror(ret), __FUNCTION__, __LINE__);
        return 0xDEADBEEF;
    }

    return data;
}