#include <unistd.h>
typedef unsigned char byte;


//#
//## Memory I/O functionality
//#

int read_bytes(size_t len, byte * source, byte * destination);
int write_bytes(size_t len, byte * destination, byte * value);

byte read_byte(byte * address);
int write_byte(byte * address, byte data);

unsigned long read_ulong(unsigned long * address);
int write_ulong(unsigned long * address, unsigned long value);

unsigned read_uint(unsigned * address);
int write_uint(unsigned * address, unsigned data);

double read_double(double * address);
int write_double(double * address, double value);

float read_single(float * address);
int write_single(float * address, float value);

int read_string(const char * address, char * buffer);
int write_string(void * address, const char * str);



unsigned long align(unsigned long value);

unsigned long reverse64(unsigned long little);
unsigned reverse32(unsigned little);
unsigned short reverse16(unsigned short little);
