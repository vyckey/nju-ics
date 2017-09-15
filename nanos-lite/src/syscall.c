#include "common.h"
#include "syscall.h"
#include "fs.h"

int _sys_none();
ssize_t _sys_write(int fd, const void *buf, size_t count);
void _sys_exit(int code);
int _sys_brk(uintptr_t addr);

_RegSet* do_syscall(_RegSet *r) {
    uintptr_t a[4];
    a[0] = SYSCALL_ARG1(r);

    switch (a[0]) {
        case SYS_none: SYSCALL_ARG1(r) = _sys_none(); break;
        case SYS_open: SYSCALL_ARG1(r) = fs_open((const char*)0 + SYSCALL_ARG2(r), SYSCALL_ARG3(r),
                SYSCALL_ARG4(r)); break;
        case SYS_read: SYSCALL_ARG1(r) = fs_read(SYSCALL_ARG2(r), (void*)0 + SYSCALL_ARG3(r),
                SYSCALL_ARG4(r)); break;
        case SYS_write: SYSCALL_ARG1(r) = fs_write(SYSCALL_ARG2(r), (const void*)0 + SYSCALL_ARG3(r),
                SYSCALL_ARG4(r)); break;
        /*case SYS_write: SYSCALL_ARG1(r) = _sys_write(SYSCALL_ARG2(r), 
                (void*)0 + SYSCALL_ARG3(r), SYSCALL_ARG4(r)); break;*/
        case SYS_close: SYSCALL_ARG1(r) = fs_close(SYSCALL_ARG2(r)); break;
        case SYS_lseek: SYSCALL_ARG1(r) = fs_lseek(SYSCALL_ARG2(r), SYSCALL_ARG3(r),
                SYSCALL_ARG4(r)); break;
        case SYS_exit: _sys_exit(SYSCALL_ARG2(r)); break;
        case SYS_brk: SYSCALL_ARG1(r) = _sys_brk(SYSCALL_ARG2(r)); break;
        default: panic("Unhandled syscall ID = %d", a[0]);
    }
    return NULL;
}

int _sys_none() {
	return 1;
}

void _sys_exit(int code) {
    _halt(code);
}

int _sys_brk(uintptr_t addr) {
    return 0;
}
/*
ssize_t _sys_write(int fd, const void *buf, size_t count) {
    const char *str = (const char*)buf;
    if(fd == 1 || fd == 2) {
        for (int i = 0; i < count; ++i) {
            _putc(str[i]);
        }
        return count;
    }
    return -1;
}*/