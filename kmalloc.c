#include "kmalloc.h"
#include "managers.h"

typedef struct { size_t size; } kmalloc_page_hdr;

typedef struct {

  void *pointer;
  size_t size;
} kmalloc_info;

uint32_t k_pages_base_addr;

// Allocate a set of pages and maintain a list of them, each page contains a
// kmalloc_page_hdr header to track how much of the page is in use, kernel
// RAM
// is from virtual addresses 0x20000000 to 0x40000000 without regard for the
// associated physical page address

void *kmalloc(size_t size) {}

void kfree(void *addr) {}
