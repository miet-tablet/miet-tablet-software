/*
 * stdio_uart_syscalls.h
 *
 *  Created on: Nov 10, 2026
 *      Author: Администратор
 */

#ifndef CORE_INC_STDIO_UART_SYSCALLS_H_
#define CORE_INC_STDIO_UART_SYSCALLS_H_

#include <sys/stat.h>
#include <errno.h>
#include <stdint.h>
#include "usart.h"

int _write(int file, char *ptr, int len);
int _read(int file, char *ptr, int len);
void* _sbrk(int incr);
int _close(int file);
int _fstat(int file, struct stat *st);
int _isatty(int file);
int _lseek(int file, int ptr, int dir);
int _open(const char *name, int flags, int mode);
int _kill(int pid, int sig);
int _getpid(void);

#endif /* CORE_INC_STDIO_UART_SYSCALLS_H_ */
