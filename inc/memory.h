#include "include.h"






pid_t find_pid(const char * process_name, pid_t pids[]);

int set_dbg_perms();


typedef struct nd_game nd_game_t;
int attach(nd_game_t nd_game);
int detach(nd_game_t * nd_game, const char * from);
int proc_wait(nd_game_t * md_game);

int proc_suspend(pid_t pid);
int proc_resume(pid_t pid);

int proc_stop(pid_t pid);
int proc_continue(pid_t pid, void * resume);

int proc_get_regs(pid_t pid, struct reg * regs);
int proc_set_regs(pid_t pid, struct reg * regs);

int proc_get_dbregs(pid_t pid, struct reg * dbregs);
int proc_set_dbregs(pid_t pid, struct reg * dbregs);

int set_watchpoint(pid_t pid, short index, void * address, int wp_type, int wp_len);
int clear_watchpoint(pid_t pid, short index);

int proc_call(nd_game_t * nd_game, void * function_address, unsigned long * return_val_ptr, struct reg * arg_regs);

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

int read_string(int pid, const char * address, char * buffer);
int write_string(int pid, void * address, const char * str);

