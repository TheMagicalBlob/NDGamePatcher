
//#
//## Debugging replacements for I/O functions. Things keep failing seemingly at random, and I can't reproduce the issues long enough to debug them. This should be helpful
//#

#define read_bytes(pid, len, source, destination) read_bytes(__FUNCTION__, __LINE__, pid, len, source, destination)
#define write_bytes(pid, len, destination, value) write_bytes(__FUNCTION__, __LINE__, pid, len, destination, value)

#define read_byte(pid, address) read_byte(__FUNCTION__, __LINE__, pid, address)
#define write_byte(pid, address, data) write_byte(__FUNCTION__, __LINE__, pid, address, data)

#define read_ulong(pid, address) read_ulong(__FUNCTION__, __LINE__, pid, address)
#define write_ulong(pid, address, value) write_ulong(__FUNCTION__, __LINE__, pid, address, value)

#define read_uint(pid, address) read_uint(__FUNCTION__, __LINE__, pid, address)
#define write_uint(pid, address, data) write_uint(__FUNCTION__, __LINE__, pid, address, data)

#define read_double(pid, adddress) read_double(__FUNCTION__, __LINE__, pid, address)
#define write_double(pid, address, value) write_double(__FUNCTION__, __LINE__, pid, address, value)

#define read_single(pid, address) read_single(__FUNCTION__, __LINE__, pid, address)
#define write_single(pid, address, value) write_single(__FUNCTION__, __LINE__, pid, address, value)

#define read_string(pid, address, buffer) read_string(__FUNCTION__, __LINE__, pid, address, buffer)
#define write_string(pid, address, str) write_string(__FUNCTION__, __LINE__, pid, address, str)
