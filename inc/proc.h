#include "include.h"


//#
//## Process manipulation/debugging functionality
//#

int set_authid_for_ptrace();


int attach(ndgame_t * ndgame);
int detach(ndgame_t * ndgame, const char * from);
pid_t proc_wait(ndgame_t * ndgame, int wait);

// int _proc_continue(pid_t pid, void * resume, int signal);

int proc_continue(ndgame_t * ndgame);
int proc_stop(ndgame_t * ndgame);

int proc_get_regs(ndgame_t * ndgame, struct reg * regs);
int proc_set_regs(ndgame_t * ndgame, struct reg * regs);

int proc_get_dbregs(ndgame_t * ndgame, struct dbreg * dbregs);
int proc_set_dbregs(ndgame_t * ndgame, struct dbreg * dbregs);

int set_watchpoint(ndgame_t * ndgame, short index, void * address, int wp_type, int wp_len);
int clear_watchpoint(ndgame_t * ndgame, short index);

int clear_args(struct reg *);
int set_args(struct reg * regs, uintptr_t rdi, uintptr_t rsi, uintptr_t rdx, uintptr_t rcx, uintptr_t rbx, uintptr_t r8, uintptr_t r9);

int proc_call(ndgame_t * ndgame, void * function_address, unsigned long * return_val_ptr, struct reg * arg_regs);

void * proc_malloc(ndgame_t * ndgame, size_t amount);
int proc_free(ndgame_t * game, void * mem_ptr);
