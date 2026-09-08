#include "inc/include.h"
// #include "inc/memory.h"


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


#ifdef DEBUG
__always_inline
static int proc_read(const char * function, int line, pid_t pid, size_t len, void * source, void * buffer)
#else
__always_inline
static int proc_read(pid_t pid, size_t len, void * source, void * buffer)
#endif
{
    errno = 0;

    if (!source)
    {
        echo("ERROR:\n# %s:%d->ptrace PT_IO (read) source address was null (src addr: 0x%llX)", function, line, source);
        return errno = EFAULT;
    }
    if (!buffer)
    {
        echo("ERROR:\n# %s:%d->ptrace PT_IO (read) destination address was null (buffer addr: 0x%llX)", function, line, buffer);
        return errno = EFAULT;
    }



    
    struct ptrace_io_desc iod = { PIOD_READ_D, source, buffer, len };


    int ret;
    if ((ret = ptrace(PT_IO, pid, (caddr_t) &iod, null)) != 0)
    {
        echo("ERROR:\n# %s:%d->ptrace PT_IO (read) failed with code %d (%s)\n\n# Source address: %llX\n# buffer address: %llX\n", function, line, ret, strerror(ret), source, buffer);
        errno = EIO;
    }


    #ifdef DEBUG
    echo("Read %d bytes from 0x%llX", iod.piod_len, source);
    #endif
    return iod.piod_len;
}




#ifdef DEBUG
__always_inline
static int proc_write(const char * function, int line, pid_t pid, size_t len, void * dest, void * value)
#else
__always_inline
static int proc_write(pid_t pid, size_t len, void * dest, void * value)
#endif
{
    if (!dest)
    {
        echo("ERROR:\n# %s:%d->ptrace PT_IO (write) destination address was invalid (src addr: 0x%llX)", function, line, dest);
        return errno = EFAULT;
    }
    if (!value)
    {
        echo("ERROR:\n# %s:%d->ptrace PT_IO (write) value address was invalid (val addr: 0x%llX)", function, line, value);
        return errno = EFAULT;
    }
    
    
    struct ptrace_io_desc iod = { PIOD_WRITE_D, dest, value, len };

    int ret;
    if ((ret = ptrace(PT_IO, pid, (caddr_t) &iod, 0)) != 0)
    {
        echo("ERROR:\n# %s:%d->ptrace PT_IO (write) failed with code %d (%s)\n\n# Destination address: %llX\n# Value Pointer: %llX (value as ulong: %llX)\n", function, line, ret, strerror(ret), dest, value, *(unsigned long *) value);
        errno = EIO;
    }

    
    #ifdef DEBUG
    echo("Wrote %d bytes to 0x%llX", iod.piod_len, dest);
    #endif
    return iod.piod_len;
}

#ifdef DEBUG
#define proc_write(pid, len, dest, value) proc_write(function, line, pid, len, dest, value)
#define proc_read(pid, len, source, buffer) proc_read(function, line, pid, len, source, buffer)
#endif








#ifdef DEBUG
int read_bytes(const char * function, int line, pid_t pid, size_t len, byte * source, byte * destination)
#else
int read_bytes(pid_t pid, size_t len, byte * source, byte * destination)
#endif
{
    return proc_read(pid, len, source, destination);
}




#ifdef DEBUG
int write_bytes(const char * function, int line, pid_t pid, size_t len, byte * destination, byte * value)
#else
int write_bytes(pid_t pid, size_t len, byte * destination, byte * value)
#endif
{
    return proc_write(pid, len, destination, value);
}








#ifdef DEBUG
byte read_byte(const char * function, int line, pid_t pid, byte * address)
#else
byte read_byte(pid_t pid, byte * address)
#endif
{
    int ret;
    byte data;

    ret = proc_read(pid, 1, address, &data);
    if (ret != 1)
    {
        return -1;
    }

    return data;
}




#ifdef DEBUG
int write_byte(const char * function, int line, pid_t pid, byte * address, byte data)
#else
int write_byte(pid_t pid, byte * address, byte data)
#endif
{
    return proc_write(pid, 1, address, &data) != 1;
}








#ifdef DEBUG
unsigned long read_ulong(const char * function, int line, pid_t pid, unsigned long * address)
#else
unsigned long read_ulong(pid_t pid, unsigned long * address)
#endif
{
    unsigned long data;

    if (proc_read(pid, 8, address, &data))
    {
        return errno = -1;
    }

    return data;
}




#ifdef DEBUG
int write_ulong(const char * function, int line, pid_t pid, unsigned long * address, unsigned long value)
#else
int write_ulong(pid_t pid, unsigned long * address, unsigned long value)
#endif
{
    return proc_write(pid, 8, address, &value) != 8;
}








#ifdef DEBUG
unsigned read_uint(const char * function, int line, pid_t pid, unsigned * address)
#else
unsigned read_uint(pid_t pid, unsigned * address)
#endif
{
    unsigned data;

    if (proc_read(pid, 4, address, &data))
    {
        return errno = -1;
    }

    return data;
}




#ifdef DEBUG
int write_uint(const char * function, int line, pid_t pid, unsigned * address, unsigned value)
#else
int write_uint(pid_t pid, unsigned * address, unsigned value)
#endif
{
    return proc_write(pid, 4, address, &value) != 4;
}








#ifdef DEBUG
double read_double(const char * function, int line, pid_t pid, double * address)
#else
double read_double(pid_t pid, double * address)
#endif
{
    double data;

    if (proc_read(pid, 8, address, &data))
    {
        return errno = -1;
    }

    return data;
}




#ifdef DEBUG
int write_double(const char * function, int line, pid_t pid, double * address, double value)
#else
int write_double(pid_t pid, double * address, double value)
#endif
{
    return proc_write(pid, 8, address, &value) != 8;
}








#ifdef DEBUG
float read_single(const char * function, int line, pid_t pid, float * address)
#else
float read_single(pid_t pid, float * address)
#endif
{
    float data;

    if (proc_read(pid, 4, address, &data))
    {
        return errno = -1;
    }

    return data;
}




#ifdef DEBUG
int write_single(const char * function, int line, pid_t pid, float * address, float value)
#else
int write_single(pid_t pid, float * address, float value)
#endif
{
    return proc_write(pid, 4, address, &value) != 4;
}








// Read a null-terminated string of characters from the traced process' memory from the provided address.
// Returns the amount of characters written, or 0 if there were any errors such as a null string or buffer pointer, or an I/O issue
#ifdef DEBUG
int read_string(const char * function, int line, pid_t pid, const char * address, char * buffer)
#else
int read_string(pid_t pid, const char * address, char * buffer)
#endif
{
    if (address == 0)
    {
        echo("null string pointer provided.");
        strncpy("(null)", buffer, 7);
        return -1;
    }
    int ret;
    unsigned len;
    
    ret = proc_read(pid, 1, (void *) address, buffer);
    __rerrv(ret != 0, strbufbuild("ERROR: Failed checking first character of string at 0x%llX before reading full string, aborting %s", address, __FUNCTION__), -1);
    if (buffer == 0) 
    {
        echo("Empty string at address 0x%llX.", (unsigned long) address);
        return 0;
    }
    
    
    len = 0;
    do {
        ret = proc_read(pid, 1, (void *) address, buffer + len ++);
        __rerrv(ret != 0, strbufbuild("ERROR: Failed reading character #%d at 0x%llX when reading string, %s aborting.", len, address, __FUNCTION__), -1);
    }
    while(*(char *) (buffer + (len - 1)));

    return len;
}



// Write a null-terminated string of characters to the traced process' memory.
// Returns the amount of characters written, or -1 if a null pointer was provided for either the source or the destination
#ifdef DEBUG
int write_string(const char * function, int line, pid_t pid, void * address, const char * str)
#else
int write_string(pid_t pid, void * address, const char * str)
#endif
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
    
    // Write string to child process' memory
    int ret;
    ret = proc_write(pid, len, (void *) address, (void *) str);
    __rerrv(ret != len, strbufbuild("ERROR: Failed writing string %s to 0x%llX, %s aborting.", len, address, __FUNCTION__), -1);

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
        buf += 16ul;
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
