#include <unistd.h>
typedef unsigned char byte;


//#
//## Memory I/O functionality
//#

int read_bytes(pid_t pid, size_t len, byte * source, byte * destination);
int write_bytes(pid_t pid, size_t len, byte * destination, byte * value);

byte read_byte(int pid, byte * address);
int write_byte(int pid, byte * address, byte data);

unsigned long read_ulong(int pid, unsigned long * address);
int write_ulong(int pid, unsigned long * address, unsigned long value);

unsigned read_uint(int pid, unsigned * address);
int write_uint(int pid, unsigned * address, unsigned data);

double read_double(int pid, double * address);
int write_double(int pid, double * address, double value);

float read_single(int pid, float * address);
int write_single(int pid, float * address, float value);

int read_string(int pid, const char * address, char * buffer);
int write_string(int pid, void * address, const char * str);

unsigned long align(unsigned long value);

unsigned long reverse64(unsigned long little);
unsigned reverse32(unsigned little);
unsigned short reverse16(unsigned short little);

