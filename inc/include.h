#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include <fcntl.h>

#include <sys/syscall.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/sysctl.h>

#include <ps5/klog.h>
#include <ps5/kernel.h>


#define null NULL
#define nullptr (void *) 0

#define TRUE 1
#define FALSE 0
#define true TRUE
#define false FALSE
