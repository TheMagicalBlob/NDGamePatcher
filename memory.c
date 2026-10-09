#include "inc/include.h"

typedef struct CallDebugger_s
{
    const char * File;   // The File of the initial caller
    const char * Caller; // Name of the function which called asserting function or handle for the asserting function
    const char * Handle; // The handle function used by the caller
    int Line;            // The line in caller from which the handle was called
    
} CallDebugger_t;


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


__always_inline
static int proc_read(pid_t pid, size_t len, void * source, void * buffer, const char * Caller)
{
    errno = 0;

    if (!source)
    {
        AssertPushBufferedMsg("%s->proc_read: source address \"0x%llX\" was null", Caller, source);
        __err(!source, NULL);
        errno = EFAULT;
        return -1;
    }
    if (!buffer)
    {
        AssertPushBufferedMsg("%s->proc_read: destination address \"0x%llX\" was null", Caller, buffer);
        __err(!buffer, NULL);
        errno = EFAULT;
        return -1;
    }
    if (pid == 0 || pid == -1)
    {
        AssertPushBufferedMsg("%s->proc_read: provided process id (%d) was invalid", Caller, pid);
        __err(pid == 0 || pid == -1, NULL);
        errno = EINVAL;
        return -1;
    }


    
    int ret;
    struct ptrace_io_desc iod = { PIOD_READ_D, source, buffer, len };


    ret = ptrace(PT_IO, pid, (caddr_t) &iod, null);
    if (ret != 0)
    {
        AssertPushBufferedMsg("%s->proc_read failed (%s)\n***     Source Address: 0x%llX\n***     Buffer Address: 0x%llX\n***", Caller, strerror(ret), source, buffer);
        __err(ret != 0, NULL);
        errno = EIO;
        return -1;
    }


    #ifdef IODEBUG
    echo("Read %d bytes from 0x%llX", iod.piod_len, source);
    #endif
    return iod.piod_len;
}




__always_inline
static int proc_write(pid_t pid, size_t len, void * dest, void * value, const char * Caller)
{
    #ifndef DEBUG
    int line;
    const char * Caller;
    Caller = "";
    Line = -1;
    #endif

    errno = 0;


    if (!dest)
    {
        AssertPushBufferedMsg("ERROR:\n# %s->proc_write: destination address was invalid (src addr: 0x%llX)", Caller, -1, dest);
        __err(!dest, NULL);
        errno = EFAULT;
        return -1;
    }
    if (!value)
    {
        AssertPushBufferedMsg("ERROR:\n# %s->proc_write: value address was invalid (val addr: 0x%llX)", Caller, -1, value);
        __err(!value, NULL);
        errno = EFAULT;
        return -1;
    }
    if (pid == 0 || pid == -1)
    {
        AssertPushBufferedMsg("ERROR:\n# %s->proc_write: provided process id was invalid (pid: %d)", Caller, -1, pid);
        __err(pid == 0 || pid == -1, NULL);
        // errno = ESRCH;
        errno = EINVAL;
        return -1;
    }


    
    int ret;
    struct ptrace_io_desc iod = { PIOD_WRITE_D, dest, value, len };

    ret = ptrace(PT_IO, pid, (caddr_t) &iod, 0);
    if (ret != 0)
    {
        AssertPushBufferedMsg("ERROR:\n# %s:%d->ptrace PT_IO (write) failed with code %d (%s)\n\n# Destination address: %llX\n# Value Pointer: %llX (value as ulong: %llX)\n", Caller, -1, ret, strerror(ret), dest, value, *(unsigned long *) value);
        __err(ret != 0, NULL);
        errno = EIO;
        return -1;
    }

    
    #ifdef IODEBUG
    echo("Wrote %d bytes to 0x%llX", iod.piod_len, dest);
    #endif
    return iod.piod_len;
}









int read_bytes(pid_t pid, size_t len, byte * source, byte * destination)
{
    return proc_read(pid, len, source, destination, __FUNCTION__);
}




int write_bytes(pid_t pid, size_t len, byte * destination, byte * value)
{
    return proc_write(pid, len, destination, value, __FUNCTION__);
}








byte read_byte(pid_t pid, byte * address)
{
    byte data;

    if (proc_read(pid, sizeof(byte), address, &data, __FUNCTION__) != sizeof(byte))
    {
        return -1;
    }

    return data;
}




int write_byte(pid_t pid, byte * address, byte data)
{
    return proc_write(pid, sizeof(byte), address, &data, __FUNCTION__) != sizeof(byte);
}








unsigned long read_ulong(pid_t pid, unsigned long * address)
{
    unsigned long data;

    if (proc_read(pid, sizeof(unsigned long), address, &data, __FUNCTION__) != sizeof(unsigned long))
    {
        return -1;
    }

    return data;
}




int write_ulong(pid_t pid, unsigned long * address, unsigned long value)
{
    return proc_write(pid, sizeof(unsigned long), address, &value, __FUNCTION__) != sizeof(unsigned long);
}








unsigned read_uint(pid_t pid, unsigned * address)
{
    unsigned data;

    if (proc_read(pid, sizeof(unsigned), address, &data, __FUNCTION__) != sizeof(unsigned))
    {
        return -1;
    }

    return data;
}




int write_uint(pid_t pid, unsigned * address, unsigned value)
{
    return proc_write(pid, sizeof(unsigned), address, &value, __FUNCTION__) != sizeof(unsigned);
}








double read_double(pid_t pid, double * address)
{
    double data;

    if (proc_read(pid, sizeof(double), address, &data, __FUNCTION__) != sizeof(double))
    {
        return -1;
    }

    return data;
}




int write_double(pid_t pid, double * address, double value)
{
    return proc_write(pid, sizeof(double), address, &value, __FUNCTION__) != sizeof(double);
}








float read_single(pid_t pid, float * address)
{
    float data;

    if (proc_read(pid, sizeof(float), address, &data, __FUNCTION__) != sizeof(float))
    {
        return -1;
    }

    return data;
}




int write_single(pid_t pid, float * address, float value)
{
    return proc_write(pid, sizeof(float), address, &value, __FUNCTION__) != sizeof(float);
}








// Read a null-terminated string of characters from the traced process' memory from the provided address.
// Returns the amount of characters written, or 0 if there were any errors such as a null string or buffer pointer, or an I/O issue
int read_string(pid_t pid, const char * source, char * buffer)
{
    if (source == 0)
    {
        echo("null string pointer provided.");
        strncpy("(null)", buffer, 7);
        return -1;
    }
    int ret;
    unsigned len;

    ret = proc_read(pid, 1, (void *) source, buffer, __FUNCTION__);
    __rerrv(ret != 1, strbufbuild("ERROR: Failed checking first character of string at 0x%llX before reading full string, aborting %s", source, __FUNCTION__), -1);

    if (buffer == 0) 
    {
        echo("Empty string at address 0x%llX.", (unsigned long) source);
        return 0;
    }
    
    
    len = 0;
    do {
        ret = proc_read(pid, 1, (void *) source + len, buffer + len, __FUNCTION__);
        __rerrv(ret != 1, strbufbuild("ERROR: Failed reading character #%d at 0x%llX when reading string, %s aborting.", len + 1, source, __FUNCTION__), -1);
    }
    while(*(char *) (buffer + (++len - 1)));

    return len;
}



// Write a null-terminated string of characters to the traced process' memory.
// Returns the amount of characters written, or -1 if a null pointer was provided for either the source or the destination
int write_string(pid_t pid, void * dest, const char * str)
{
    if (!dest) {
        echo("null destination string pointer provided.");
        return -1;
    }

    if (!str) {
        echo("null source string pointer provided.");
        return -1;
    }

    if (!*str) 
    {
        echo("Empty string at address 0x%llX.", (unsigned long long) dest);
        return -1;
    }
    
 
    // Get string length
    int len = 0;
    while (*(str + ++len) != 0);
    
    // Write string to child process' memory
    int ret;
    ret = proc_write(pid, len, (void *) dest, (void *) str, __FUNCTION__);
    __rerrv(ret != len, strbufbuild("ERROR: Failed writing string %s to 0x%llX, %s aborting.", len, dest, __FUNCTION__), -1);

    return len;
}








// Lazily align the provided by rounding it up to the nearest tenth (unless it's already aligned to 16 bytes).
// This isn't really needed right now, but I'm keeping it just-in-case
unsigned long align(unsigned long value)
{
    unsigned long buf;
    buf = value & ~0xF;

    if (buf < value)
    {
        buf += 0x10ul;
    }

    return buf;
}








unsigned long reverse64(unsigned long little)
{
    __asm__(".intel_syntax noprefix\n"
        "bswap rdi\n"
        "mov rax, rdi\n"
        "ret\n"
    );

    return 0xDEADA55;
}




unsigned reverse32(unsigned little)
{
    __asm__(".intel_syntax noprefix\n"
        "bswap edi\n"
        "mov eax, edi\n"
        "ret\n"
    );

    return 0xDEADA55;
}




unsigned short reverse16(unsigned short little)
{
    __asm__(".intel_syntax noprefix\n"
        "shl edi, 0x10\n"
        "bswap edi\n"
        "mov ax, di\n"
        "ret\n"
    );

    return 0xDEAD;
}
