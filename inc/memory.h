#include "include.h"

#define _SHELLCORE_AUTHID 0x4800000000010003L




int set_dbg_perms();

pid_t find_pid(const char * process_name);

int attach(int pid);
int detach(int pid);

void * proc_malloc(pid_t pid, int amount);
int proc_free(void * mem_ptr);

int read_bytes(int pid, size_t length, void * address, void * buffer);
int read_string(int pid, void * address, void * buffer);
unsigned long long read_ulong(int pid, unsigned long * address);
unsigned int read_uint(int pid, void * address);
unsigned char read_byte(int pid, void * address);

int write_ulong(pid_t pid, unsigned long * address, unsigned long value);

