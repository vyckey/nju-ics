#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void ramdisk_read(void *buf, off_t offset, size_t len);
void ramdisk_write(const void *buf, off_t offset, size_t len);
size_t fb_write(const void *buf, off_t offset, size_t len);
size_t dispinfo_read(void *buf, off_t offset, size_t len);
size_t events_read(void *buf, size_t len);

extern const size_t dispinfo_bytes;

void init_fs() {
    // TODO: initialize the size of /dev/fb
    file_table[FD_FB].size = (_screen.width * _screen.height) << 2;
    file_table[FD_DISPINFO].size = dispinfo_bytes;
}

int fs_open(const char *pathname, int flags, int mode) {
    for (int i = 0; i < NR_FILES; ++i) {
        if (strcmp(file_table[i].name, pathname) == 0) {
            file_table[i].open_offset = 0;
            return i;
        }
    }
    printf("File %s doesn't exist!\n", pathname);
    assert(0);
    return -1;
}

size_t fs_filesz(int fd) {
    return (0 <= fd && fd < NR_FILES) ? file_table[fd].size : 0;
}

ssize_t fs_read(int fd, void *buf, size_t len) {
    size_t read_len = -1;
    if (fd == FD_STDIN || fd == FD_STDOUT || fd == FD_STDERR) read_len = 0;
    else if (fd == FD_FB) ;
    else if (fd == FD_EVENTS) return events_read(buf, len);
    else if (0 <= fd && fd < NR_FILES) {
        Finfo *f = &file_table[fd];
        read_len = (f->open_offset + len > f->size) ? (f->size - f->open_offset) : len;
        if (fd == FD_DISPINFO) dispinfo_read(buf, f->open_offset, read_len);
        else ramdisk_read(buf, f->disk_offset + f->open_offset, read_len);
        f->open_offset += read_len;
    }
    return read_len;
}

ssize_t fs_write(int fd, const void *buf, size_t len) {
    size_t write_len = -1;
    if (fd == FD_STDOUT || fd == FD_STDERR) {
        const char *str = (const char*)buf;
        for (int i = 0; i < len; ++i) {
            _putc(str[i]);
        }
        write_len = len;
    }
    else if (fd == FD_STDIN || fd == FD_DISPINFO || fd == FD_EVENTS) ;
    else if (0 <= fd && fd < NR_FILES) {
        Finfo *f = &file_table[fd];
        write_len = (f->open_offset + len > f->size) ? (f->size - f->open_offset) : len;
        if (fd == FD_FB) fb_write(buf, f->open_offset, write_len);
        else ramdisk_write(buf, f->disk_offset + f->open_offset, write_len);
        f->open_offset += write_len;
    }
    return write_len;
}

off_t fs_lseek(int fd, off_t offset, int whence) {
    if (fd == FD_STDIN || fd == FD_STDOUT || fd == FD_STDERR) {assert(0); return 0;}
    else if (0 <= fd && fd < NR_FILES) {
        Finfo *f = &file_table[fd];
        if (whence == SEEK_SET) {//printf("%d %d\n", f->open_offset, offset);
            if (0 <= offset && offset <= f->size) return (f->open_offset = offset);
            return EOVERFLOW; 
        }
        else if (whence == SEEK_CUR) {
            off_t next_off = f->open_offset + offset;
            if (0 <= next_off && next_off <= f->size) return (f->open_offset = next_off);
            return EOVERFLOW;
        }
        else if (whence == SEEK_END) {
            off_t next_off = f->size + offset;
            if (0 <= next_off && next_off <= f->size) return (f->open_offset = next_off);
            return EOVERFLOW;
        }
        return EINVAL;
    }
    return EBADF;
}

int fs_close(int fd) {
    return 0;
}