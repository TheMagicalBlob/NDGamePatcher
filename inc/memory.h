#include "include.h"






pid_t find_pid(const char * process_name, pid_t pids[]);

int set_authid_for_ptrace();


int attach(ndgame_t nd_game);
int detach(ndgame_t * nd_game, const char * from);
pid_t proc_wait(ndgame_t * game, int wait);

// int _proc_continue(pid_t pid, void * resume, int signal);

int proc_continue(pid_t pid);
int proc_stop(pid_t pid);

int proc_get_regs(pid_t pid, struct reg * regs);
int proc_set_regs(pid_t pid, struct reg * regs);

int proc_get_dbregs(pid_t pid, struct dbreg * dbregs);
int proc_set_dbregs(pid_t pid, struct dbreg * dbregs);

int set_watchpoint(pid_t pid, short index, void * address, int wp_type, int wp_len);
int clear_watchpoint(pid_t pid, short index);

int clear_args(struct reg *);
int set_args(struct reg * regs, uintptr_t rdi, uintptr_t rsi, uintptr_t rdx, uintptr_t rcx, uintptr_t rbx, uintptr_t r8, uintptr_t r9);

int proc_call(ndgame_t * nd_game, void * function_address, unsigned long * return_val_ptr, struct reg * arg_regs);

void * proc_malloc(pid_t pid, int amount);
int proc_free(pid_t pid, void * mem_ptr);

int read_bytes(pid_t pid, size_t len, void * address, void * buffer);
int write_bytes(pid_t pid, size_t len, void * address, void * buffer);

unsigned char read_byte(int pid, void * address);
int write_byte(int pid, void * address, unsigned char data);

unsigned long read_ulong(int pid, unsigned long * address);
int write_ulong(int pid, unsigned long * address, unsigned long value);

unsigned int read_uint(int pid, void * address);
int write_uint(int pid, void * address, unsigned int data);

double read_double(int pid, void * address);
int write_double(int pid, void * address, double value);

float read_single(int pid, unsigned long address);
int write_single(int pid, unsigned long address, float value);

int read_string(int pid, const char * address, char * buffer);
int write_string(int pid, void * address, const char * str);

