#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <sys/stat.h>
#include <errno.h>
#include <stdint.h>
#include "stdio_uart.h"

int _write(int file, char *ptr, int len);
int _read(int file, char *ptr, int len);
void *_sbrk(int incr);
int _close(int file);
int _fstat(int file, struct stat *st);
int _isatty(int file);
int _lseek(int file, int ptr, int dir);
int _open(const char *name, int flags, int mode);
int _kill(int pid, int sig);
int _getpid(void);

#if defined(__cplusplus)
}
#endif