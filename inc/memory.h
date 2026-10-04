#include <unistd.h>
typedef unsigned char byte;


//#
//## Memory I/O functionality
//#
typedef struct CallDebugger_s CallDebugger_t;



int read_bytes(pid_t pid, size_t len, byte * source, byte * destination);
int write_bytes(pid_t pid, size_t len, byte * destination, byte * value);

byte read_byte(pid_t pid, byte * address);
int write_byte(pid_t pid, byte * address, byte data);

unsigned long read_ulong(pid_t pid, unsigned long * address);
int write_ulong(pid_t pid, unsigned long * address, unsigned long value);

unsigned read_uint(pid_t pid, unsigned * address);
int write_uint(pid_t pid, unsigned * address, unsigned data);

double read_double(pid_t pid, double * address);
int write_double(pid_t pid, double * address, double value);

float read_single(pid_t pid, float * address);
int write_single(pid_t pid, float * address, float value);

int read_string(pid_t pid, const char * address, char * buffer);
int write_string(pid_t pid, void * address, const char * str);



unsigned long align(unsigned long value);

unsigned long reverse64(unsigned long little);
unsigned reverse32(unsigned little);
unsigned short reverse16(unsigned short little);
