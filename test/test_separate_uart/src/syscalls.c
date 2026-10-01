#include <sys/stat.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>

// Ваши функции, которые вы уже реализовали
extern void my_putc(char c);
extern char my_getc(void);

// Указатель на конец кучи (для работы malloc)
extern uint32_t _end;       // Конец RAM (определяется в linker script)
extern uint32_t _estack;    // Верхушка стека (определяется в linker script)
static uint32_t* heap_ptr = NULL;

#define STDOUT_FILENO       1
#define STDERR_FILENO       2
#define STDIN_FILENO        0

// ============================================================
// 1. ВЫВОД (printf, puts, fwrite)
// ============================================================
int _write(int file, char *ptr, int len) {
    if (file == STDOUT_FILENO || file == STDERR_FILENO) {
        for (int i = 0; i < len; i++) {
            my_putc(ptr[i]);
        }
        return len;
    }
    errno = EBADF;
    return -1;
}

// ============================================================
// 2. ВВОД (scanf, fgets)
// ============================================================
int _read(int file, char *ptr, int len) {
    if (file == STDIN_FILENO) {
        int n = 0;
        while (n < len) {
            char c = my_getc();
            ptr[n++] = c;
            if (c == '\n' || c == '\r') break; // Стоп по Enter
        }
        return n;
    }
    errno = EBADF;
    return -1;
}

// ============================================================
// 3. ЗАГЛУШКИ ДЛЯ ОСТАЛЬНЫХ СИСТЕМНЫХ ВЫЗОВОВ
// (Закрывают все ошибки линкера из вашего лога)
// ============================================================

// Выделение памяти (malloc, calloc, realloc)
void* _sbrk(int incr) {
    if (heap_ptr == NULL) {
        heap_ptr = (uint32_t*)&_end;
    }
    uint32_t* prev_heap_ptr = heap_ptr;
    
    // Проверка, не залезли ли мы в стек
    if ((uint32_t)(heap_ptr + incr) > (uint32_t)&_estack) {
        errno = ENOMEM;
        return (void*)-1; // Память закончилась
    }
    
    heap_ptr += incr;
    return (void*)prev_heap_ptr;
}

// Закрытие файла (fclose)
int _close(int file) {
    (void)file;
    return -1;
}

// Информация о файле (fstat)
int _fstat(int file, struct stat *st) {
    (void)file;
    st->st_mode = S_IFCHR; // Символьное устройство (консоль)
    return 0;
}

// Проверка, является ли файл терминалом (isatty)
int _isatty(int file) {
    if (file <= STDERR_FILENO) return 1; // stdin, stdout, stderr - это терминал
    return 0;
}

// Перемещение курсора в файле (lseek)
int _lseek(int file, int ptr, int dir) {
    (void)file; (void)ptr; (void)dir;
    return 0;
}

// Открытие файла (fopen)
int _open(const char *name, int flags, int mode) {
    (void)name; (void)flags; (void)mode;
    return -1;
}

// Отправка сигнала процессу (kill) - нужно для некоторых версий libc
int _kill(int pid, int sig) {
    (void)pid; (void)sig;
    errno = EINVAL;
    return -1;
}

// Получение PID процесса (getpid)
int _getpid(void) {
    return 1;
}