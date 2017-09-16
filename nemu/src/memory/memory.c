#include "nemu.h"
#include "device/mmio.h"
#include "memory/mmu.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
	int device_no = is_mmio(addr);
 	return ((device_no == -1)
 		? pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3))
 		: mmio_read(addr, len, device_no));
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
	int device_no = is_mmio(addr);
	if (device_no == -1)
 		memcpy(guest_to_host(addr), &data, len);
 	else mmio_write(addr, len, data, device_no);
}

uint32_t vaddr_read(vaddr_t addr, int len) {
	/*if ((addr & PAGE_MASK) + len > PAGE_SIZE) {
		TODO();
		int len1, len2;
		uint32_t data;
		paddr_t paddr;
		len1 = PAGE_SIZE - (addr & PAGE_MASK);
		len2 = len - len1;
		paddr = page_translate(addr);
		data = paddr_read(paddr, len1);
		paddr = page_translate(addr + len1);
		data = (paddr_read(paddr, len2) << len1) | data;
		return data;
	}
	else {
		paddr_t paddr = page_translate(addr);
		return paddr_read(paddr, len);
	}*/
	return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
	/*if ((addr & PAGE_MASK) + len > PAGE_SIZE) {
		int len1, len2;
		paddr_t paddr;
		len1 = PAGE_SIZE - (addr & PAGE_MASK);
		len2 = len - len1;
		paddr = page_translate(addr);
		paddr_write(paddr, len1, data);
		paddr = page_translate(addr + len1);
		paddr_write(paddr, len2, data >> len1);
	}
	else {
		paddr_t paddr = page_translate(addr);
		paddr_write(paddr, len);
	}*/
	paddr_write(addr, len, data);
}
