#include "inc/include.h"
#include "inc/dmenu.h"

extern NdGame_t * game;

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
static int proc_read(pid_t pid, size_t Length, void * Source, void * Destination, const char * Caller)
{
    errno = 0;
    const char * errMsg = "%s->proc_read failed (%x -> %s)\n***   PID:     %d\n***   Length:  %d\n***   Source Address: 0x%llX\n***   Destination Address: 0x%llX (0x%llX)\n***";

    if (!Source)
    {
        errno = EFAULT;

        __err(!Source, errMsg, Caller, "source address was null", pid, Length, Source, Destination);
        return -1;
    }
    if (!Destination)
    {
        errno = EFAULT;

        __err(!Destination, errMsg, Caller, "destination address was null", pid, Length, Source, Destination);
        return -1;
    }
    if (pid == 0 || pid == -1)
    {
        //! TODO: Implement a check for nonexistent process id's specifically
        // ^ errno = ESRCH;
        errno = EINVAL;

        __err(pid == 0 || pid == -1, errMsg, Caller, "provided process id was invalid", pid, Length, Source, Destination);
        return -1;
    }


    //! TODO: Avoid changing vmem protections when not writing to .text
    int prot = kernel_get_vmem_protection(pid, game->BaseAddress, Length);
    kernel_set_vmem_protection(pid, game->BaseAddress, Length, prot | PROT_READ);

    struct ptrace_io_desc iod = { PIOD_READ_D, Source, Destination, Length };

    int ret = ptrace(PT_IO, pid, (caddr_t) &iod, null);
    if (ret != 0)
    {
        __err(ret != 0, errMsg, Caller, ret, strerror(ret), pid, Length, Source, Destination, *(char *)Destination);
        errno = EIO;
        return -1;
    }

    kernel_set_vmem_protection(pid, game->BaseAddress, Length, prot);


    #if VERBOSITY > 2
    echo("Read %d bytes from 0x%llX", iod.piod_len, Source);
    #endif
    return iod.piod_len;
}




__always_inline
static int proc_write(pid_t pid, size_t Length, void * Destination, void * Source, const char * Caller)
{
    errno = 0;
    const char * errMsg = "%s->proc_write failed (%s)\n***   PID:     %d\n***   Length:  %d\n***   Source Address: 0x%llX\n***   Destination Address: 0x%llX\n***";

    if (!Source)
    {
        errno = EFAULT;

        __err(!Source, errMsg, Caller, "source address was null", pid, Length, Source, Destination);
        return -1;
    }
    if (!Destination)
    {
        errno = EFAULT;

        __err(!Destination, errMsg, Caller, "destination address was null", pid, Length, Source, Destination);
        return -1;
    }
    if (pid == 0 || pid == -1)
    {
        errno = EINVAL;

        __err(pid == 0 || pid == -1, errMsg, Caller, "provided process id was invalid", pid, Length, Source, Destination);
        return -1;
    }


    int prot = kernel_get_vmem_protection(pid, game->BaseAddress, Length);
    kernel_set_vmem_protection(pid, game->BaseAddress, Length, prot | PROT_READ);

    struct ptrace_io_desc iod = { PIOD_WRITE_D, Destination, Source, Length };

    int ret = ptrace(PT_IO, pid, (caddr_t) &iod, 0);
    if (ret != 0)
    {
        __err(ret != 0, errMsg, Caller, strerror(ret), pid, Length, Source, Destination);
        errno = EIO;
        return -1;
    }

    kernel_set_vmem_protection(pid, game->BaseAddress, Length, prot);

    
    #if VERBOSITY > 2
    echo("Wrote %d bytes to 0x%llX", iod.piod_len, Destination);
    #endif
    return iod.piod_len;
}









int read_bytes(size_t len, byte * source, byte * destination)
{
    return proc_read(game->pid, len, source, destination, __FUNCTION__);
}




int write_bytes(size_t len, byte * destination, byte * value)
{
    return proc_write(game->pid, len, destination, value, __FUNCTION__);
}








byte read_byte(byte * address)
{
    byte data;

    if (proc_read(game->pid, sizeof(byte), address, &data, __FUNCTION__) != sizeof(byte))
    {
        return -1;
    }

    return data;
}




int write_byte(byte * address, byte data)
{
    return proc_write(game->pid, sizeof(byte), address, &data, __FUNCTION__) != sizeof(byte);
}








unsigned long read_ulong(unsigned long * address)
{
    unsigned long data;

    if (proc_read(game->pid, sizeof(unsigned long), address, &data, __FUNCTION__) != sizeof(unsigned long))
    {
        return -1;
    }

    return data;
}




int write_ulong(unsigned long * address, unsigned long value)
{
    return proc_write(game->pid, sizeof(unsigned long), address, &value, __FUNCTION__) != sizeof(unsigned long);
}








unsigned read_uint(unsigned * address)
{
    unsigned data;

    if (proc_read(game->pid, sizeof(unsigned), address, &data, __FUNCTION__) != sizeof(unsigned))
    {
        return -1;
    }

    return data;
}




int write_uint(unsigned * address, unsigned value)
{
    return proc_write(game->pid, sizeof(unsigned), address, &value, __FUNCTION__) != sizeof(unsigned);
}








double read_double(double * address)
{
    double data;

    if (proc_read(game->pid, sizeof(double), address, &data, __FUNCTION__) != sizeof(double))
    {
        return -1;
    }

    return data;
}




int write_double(double * address, double value)
{
    return proc_write(game->pid, sizeof(double), address, &value, __FUNCTION__) != sizeof(double);
}








float read_single(float * address)
{
    float data;

    if (proc_read(game->pid, sizeof(float), address, &data, __FUNCTION__) != sizeof(float))
    {
        return -1;
    }

    return data;
}




int write_single(float * address, float value)
{
    return proc_write(game->pid, sizeof(float), address, &value, __FUNCTION__) != sizeof(float);
}








// Read a null-terminated string of characters from the traced process' memory from the provided address.
// Returns the amount of characters read, or 0 if there were any errors such as a null string or buffer pointer, or an I/O issue
int read_string(const char * source, char * buffer)
{
    if (source == 0)
    {
        echo("null string pointer provided.");
        strncpy("(null)", buffer, 7);
        return -1;
    }
    int ret;
    unsigned len;

    ret = proc_read(game->pid, 1, (void *) source, buffer, __FUNCTION__);
    __rerrv(ret != 1, -1, "ERROR: Failed checking first character of string at 0x%llX before reading full string, aborting %s", source, __FUNCTION__);

    if (buffer == 0) 
    {
        echo("Empty string at address 0x%llX.", (unsigned long) source);
        return 0;
    }
    
    
    len = 0;
    do {
        ret = proc_read(game->pid, 1, (void *) source + len, buffer + len, __FUNCTION__);
        __rerrv(ret != 1, -1, "ERROR: Failed reading character #%d at 0x%llX when reading string, %s aborting.", len + 1, source, __FUNCTION__);
    }
    while(*(char *) (buffer + (++len - 1)));

    return len;
}



// Write a null-terminated string of characters to the traced process' memory.
// Returns the amount of characters written, or -1 if a null pointer was provided for either the source or the destination
int write_string(void * dest, const char * str)
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
    ret = proc_write(game->pid, len, (void *) dest, (void *) str, __FUNCTION__);
    __rerrv(ret != len, -1, "ERROR: Failed writing string %s to 0x%llX, %s aborting.", len, dest, __FUNCTION__);

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
