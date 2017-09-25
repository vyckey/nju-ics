#include "common.h"
#include "fs.h"
#include "memory.h"

/* #define DEFAULT_ENTRY ((void *)0x4000000) */
#define DEFAULT_ENTRY ((void *)0x8048000)

size_t get_ramdisk_size();

uintptr_t loader(_Protect *as, const char *filename) {
	//ramdisk_read(DEFAULT_ENTRY, 0, get_ramdisk_size());
	int fd = fs_open(filename, 0, 0);
	if (fd != -1) {
		//fs_read(fd, DEFAULT_ENTRY, fs_filesz(fd));
		//fs_close(fd);
		/* paging address */
		void *vaddr, *paddr;
		vaddr = DEFAULT_ENTRY;
		for (size_t i = 0, sz = fs_filesz(fd); i < sz; i += PGSIZE) {
			size_t read_bytes = (sz - i >= PGSIZE) ? PGSIZE : (sz - i);
			paddr = new_page();
			_map(as, vaddr, paddr);
			vaddr = PGSIZE + (uint8_t*)vaddr;
			fs_read(fd, paddr, read_bytes);
		}
		fs_close(fd);
		/* paging address */
	}
	//return (uintptr_t)DEFAULT_ENTRY;
	return (uintptr_t)DEFAULT_ENTRY;
}
