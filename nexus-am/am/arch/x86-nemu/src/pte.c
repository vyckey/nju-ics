#include <x86.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

static PDE kpdirs[NR_PDE] PG_ALIGN;
static PTE kptabs[PMEM_SIZE / PGSIZE] PG_ALIGN;
static void* (*palloc_f)();
static void (*pfree_f)(void*);

_Area segments[] = {      // Kernel memory mappings
  {.start = (void*)0,          .end = (void*)PMEM_SIZE}
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0]))

void _pte_init(void* (*palloc)(), void (*pfree)(void*)) {
  palloc_f = palloc;
  pfree_f = pfree;

  int i;

  // make all PDEs invalid
  for (i = 0; i < NR_PDE; i ++) {
    kpdirs[i] = 0;
  }

  PTE *ptab = kptabs;
  for (i = 0; i < NR_KSEG_MAP; i ++) {
    uint32_t pdir_idx = (uintptr_t)segments[i].start / (PGSIZE * NR_PTE);
    uint32_t pdir_idx_end = (uintptr_t)segments[i].end / (PGSIZE * NR_PTE);
    for (; pdir_idx < pdir_idx_end; pdir_idx ++) {
      // fill PDE
      kpdirs[pdir_idx] = (uintptr_t)ptab | PTE_P;

      // fill PTE
      PTE pte = PGADDR(pdir_idx, 0, 0) | PTE_P;
      PTE pte_end = PGADDR(pdir_idx + 1, 0, 0) | PTE_P;
      for (; pte < pte_end; pte += PGSIZE) {
        *ptab = pte;
        ptab ++;
      }
    }
  }

  set_cr3(kpdirs);
  set_cr0(get_cr0() | CR0_PG);
}

void _protect(_Protect *p) {
  PDE *updir = (PDE*)(palloc_f());
  p->ptr = updir;
  // map kernel space
  for (int i = 0; i < NR_PDE; i ++) {
    updir[i] = kpdirs[i];
  }

  p->area.start = (void*)0x8000000;
  p->area.end = (void*)0xc0000000;
}

void _release(_Protect *p) {
}

void _switch(_Protect *p) {
  set_cr3(p->ptr);
}

void _map(_Protect *p, void *va, void *pa) {
  uint32_t vaddr = (uint32_t)va, paddr = (uint32_t)pa;
  uint32_t pde_idx = (vaddr >> PDXSHFT) & (NR_PDE - 1), pte_idx = (vaddr >> PTXSHFT) & (NR_PTE - 1);
  PDE pde = ((PDE*)(p->ptr))[pde_idx];
  if ((pde & 0x1) != 0x1) {
    uintptr_t *new_pde = palloc_f();
    for (int i = 0; i < NR_PTE; ++i) new_pde[i] = 0;
    pde = (uintptr_t)new_pde;
    ((PDE*)(p->ptr))[pde_idx] = pde | 0x1;
  }
  pde = pde & ~(PGSIZE - 1);
  ((PTE*)pde)[pte_idx] = paddr | 0x1;
}

void _unmap(_Protect *p, void *va) {
}

_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, void *entry,
                char *const argv[], char *const envp[]) {
  void *pos = ustack.end - sizeof(_RegSet) - 8;
  _RegSet *trap_frame = pos;
  trap_frame->eflags = 0x202;
  trap_frame->cs = 8;
  trap_frame->eip = (uintptr_t)entry;
  trap_frame->esp = sizeof(_RegSet) + (uintptr_t)trap_frame;
  return trap_frame;
}
