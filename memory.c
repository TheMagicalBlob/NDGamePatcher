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




int read_bytes(pid_t pid, size_t len, void * source, void * destination)
{
    return proc_read(pid, len, source, destination, __FUNCTION__);
}



int write_bytes(pid_t pid, size_t len, void * destination, void * value)
{
    return proc_write(pid, len, destination, value, __FUNCTION__);
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
        strncpy("(null)", buffer, 7);
        return -1;
    }
    

    if (read_byte(pid, (void *) address) <= 0) 
    {
        echo("Empty string at address 0x%llX.", (unsigned long long) address);
        // strncpy("(empty)", buffer, 8);
        return 0;
    }
    
    

    unsigned len = 0;
    // Read the string until it's null or we encounter an error (why did I write it this way lmao)
    do {
        *(char *) (buffer + len) = read_byte(pid, (void *) address + len);
    }
    while(!errno && ++len && *(char *) (buffer + (len-1)));

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
    
    int written = 0;
    for(long long i = 0; i < len; ++i)
    {
        if (!write_byte(pid, (void *) address + i, *(char *) (str + i)))
        {
            ++written;
        }
    }
    write_byte(pid, (void *) address + len, 0); // terminate the string

    return written;
}



// Lazily align the provided by rounding it up to the nearest tenth (unless it's already aligned to 16 bytes).
// This isn't really needed right now, but I'm keeping it just-in-case
unsigned long align(unsigned long value)
{
    unsigned long buf;
    buf = value & ~0xF;

    if (buf < value)
    {
        buf += 16ul;
    }

    return buf;
}
