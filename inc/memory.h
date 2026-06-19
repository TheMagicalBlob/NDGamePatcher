#include "include.h"

#define _SHELLCORE_AUTHID 0x4800000000010003L

#define T2_FREE 0x02a8f680
#define T2_MALLOC 0x02a8f690

#define DMENU_DEV_MENU_BASE_PTR       0x6b63998
#define DMENU_QUICK_MENU_BASE_PTR     0x6b64670
#define DMENU_FAVORITES_MENU_BASE_PTR 0x6b639a0


int set_dbg_perms();

pid_t find_pid(const char * process_name);

int attach(int pid);
int detach(int pid);

int read_bytes(int pid, size_t length, void * address, void * buffer);
unsigned long read_ulong(int pid, unsigned long address);
unsigned int read_uint(int pid, unsigned long address);
