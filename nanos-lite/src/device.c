#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
	int key;
	if ((key = _read_key()) != _KEY_NONE) {
		if (key & 0x8000) {
			key ^= 0x8000;
			sprintf(buf, "kd %s\n", keyname[key]);
		}
		else sprintf(buf, "ku %s\n", keyname[key]);
	}
	else sprintf(buf, "t %u\n", _uptime());
	return strlen(buf);
}

static char dispinfo[128] __attribute__((used));
const size_t dispinfo_bytes = sizeof(dispinfo)/sizeof(char);

size_t dispinfo_read(void *buf, off_t offset, size_t len) {
	memcpy(buf, &dispinfo[offset], len);
	return len;
}

size_t fb_write(const void *buf, off_t offset, size_t len) {
	_draw_fill(buf, offset >> 2, len >> 2);
	/*
	int x, y, h, write_bytes;
	y = offset / _screen.width;
	x = offset % _screen.width;
	write_bytes = (x + len < _screen.width) ? len : (_screen.width - x);
	_draw_rect(buf, x, y, write_bytes, 1);
	len -= write_bytes;
	h = len / _screen.width;
	_draw_rect(buf, 0, ++y, _screen.width, h);
	len %= _screen.width;
	y += h;
	_draw_rect(buf, 0, y, len, 1);
	*/
	return len;
}

void init_device() {
	_ioe_init();
	// TODO: print the string to array `dispinfo` with the format
	// described in the Navy-apps convention
	sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", _screen.width, _screen.height);
}
