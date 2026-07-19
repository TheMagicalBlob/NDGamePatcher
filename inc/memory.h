#include "include.h"




typedef enum wp_type wp_type_t;


pid_t find_pid(const char * process_name);

int set_dbg_perms();

int attach(int pid);
int detach(int pid, const char * from);
void proc_wait(pid_t pid);

int proc_suspend(pid_t pid);
int proc_continue(pid_t pid, void * resume);

int get_regs(pid_t pid, struct reg * regs);
int set_regs(pid_t pid, struct reg * regs);

int set_watchpoint(pid_t pid, short index, void * address, wp_type_t type);
int clear_watchpoint(pid_t pid, short index);

int proc_call(pid_t pid, void * function_address, unsigned long * return_val_ptr, struct reg * arg_regs);

void * proc_malloc(pid_t pid, int amount);
int proc_free(pid_t pid, void * mem_ptr);

int read_bytes(pid_t pid, size_t len, void * address, void * buffer);
int write_bytes(pid_t pid, size_t len, void * address, void * buffer);

unsigned char read_byte(int pid, void * address);
int write_byte(int pid, void * address, unsigned char data);

unsigned long long read_ulong(int pid, unsigned long * address);
int write_ulong(int pid, unsigned long * address, unsigned long value);

unsigned int read_uint(int pid, void * address);
int write_uint(int pid, void * address, unsigned int data);

double read_double(int pid, void * address);
int write_double(int pid, void * address, double value);

float read_single(int pid, unsigned long address);
int write_single(int pid, unsigned long address, float value);

int read_string(int pid, void * address, char * buffer);
int write_string(int pid, void * address, const char * str);

