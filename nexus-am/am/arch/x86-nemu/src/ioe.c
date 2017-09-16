#include <am.h>
#include <x86.h>

#define RTC_PORT 0x48   // Note that this is not standard
#define I8042_DATA_PORT 0x60
#define I8042_STATUS_PORT 0x64
static unsigned long boot_time;

void _ioe_init() {
  boot_time = inl(RTC_PORT);
}

unsigned long _uptime() {
  return inl(RTC_PORT) - boot_time;
}

uint32_t* const fb = (uint32_t *)0x40000;

_Screen _screen = {
  .width  = 400,
  .height = 300,
};

extern void* memcpy(void *, const void *, int);

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  int j, cp_bytes;
  cp_bytes = (_screen.width - x > w) ? w : (_screen.width - x);
  cp_bytes *= sizeof(uint32_t);
  for (j = 0; j < h && y + j < _screen.height; ++j) {
  	memcpy(fb + ((y + j) * _screen.width + x), pixels, cp_bytes);
  	pixels += w;
  }
}

size_t _draw_fill(const uint32_t *pixels, off_t offset, size_t plen) {
  size_t fill_pixels = (offset + plen <= _screen.width * _screen.height) ? plen
                      : (_screen.width * _screen.height - plen);
  memcpy(&fb[offset], pixels, fill_pixels << 2);
  return fill_pixels;
}

void _draw_sync() {
}

int _read_key() {
	if (inb(I8042_STATUS_PORT) == 0x1) {
		return inl(I8042_DATA_PORT);
	}
	return _KEY_NONE;
}
