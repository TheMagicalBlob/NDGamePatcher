#include "include.h"


//#
//## Memory I/O functionality
//#


int read_bytes(pid_t pid, size_t len, void * source, void * destination);
int write_bytes(pid_t pid, size_t len, void * destination, void * value);

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

unsigned long align(unsigned long value);

