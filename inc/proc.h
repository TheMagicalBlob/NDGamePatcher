#include <unistd.h>
#include <sys/ptrace.h>


//#
//## Process manipulation/debugging functionality
//#

int set_authid_for_ptrace();


int attach(pid_t pid);
int detach(const char * from);
pid_t proc_wait(int wait);

// int _proc_continue(void * resume, int signal);

int proc_continue();
int proc_stop();

int proc_get_regs(struct reg * regs);
int proc_set_regs(struct reg * regs);

int proc_get_dbregs(struct dbreg * dbregs);
int proc_set_dbregs(struct dbreg * dbregs);

int set_watchpoint(short index, void * address, int wp_type, int wp_len);
int clear_watchpoint(short index);

int proc_clear_args(struct reg * regs);
int proc_set_args(struct reg * regs, uintptr_t rdi, uintptr_t rsi, uintptr_t rdx, uintptr_t rcx, uintptr_t rbx, uintptr_t r8, uintptr_t r9);

int proc_call(void * function_address, unsigned long * return_val_ptr, struct reg * arg_regs);

void * proc_malloc(size_t amount);
int proc_free(void * mem_ptr);
