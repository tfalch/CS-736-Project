#include <linux/mm.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/linkage.h>
#include <linux/mm_types.h>
#include <linux/slab.h>
#include <linux/swap.h>
#include <linux/mmzone.h>
#include <linux/mm_inline.h>

#include "internal.h"

#ifndef DEBUG
#define DEBUG 1
#endif 

/* comment/uncomment to enable/disable DEBUG_PRINT */
#ifdef DEBUG
#undef DEBUG
#endif

#ifdef DEBUG
#define PRINT_FX_NAME printk(KERN_EMERG __FUNCTION__);
#define PRINT_LOCATION printk(KERN_EMERG __LINE__);
#define DEBUG_PRINT(fmt, ...) \
  printk(KERN_EMERG fmt, __VA_ARGS__); \
  printk(KERN_EMERG "\n");
#else
#define DEBUG_PRINT(fmt, ...)
#endif

typedef enum hate_lvl {
  DISLIKE,  // moves page to inactive list to be freed eventually
  HATE      // frees page instantly. 
} hate_intensity;;
  
static inline int __hate_page(struct page * page, enum hate_lvl hate) {

    DEBUG_PRINT("hating page: %lu\n", (unsigned long)page);
    
    if (hate == DISLIKE) {
        if (PageLRU(page)) {
	    unsigned long flags;
	    enum lru_list lru = LRU_BASE;
	    struct zone * zone = page_zone(page);
	    
	    spin_lock_irqsave(&zone->lru_lock, flags);

	    lru = page_lru_base_type(page);
	    del_page_from_lru(zone, page);
	    
	    __ClearPageActive(page);
	    __ClearPageReferenced(page);
	    
	    add_page_to_lru_list(zone, page, lru);

	    spin_unlock_irqrestore(&zone->lru_lock, flags);
	}
    } else {
        put_page(page); 
    }
	     
    
    /*
    ClearPageReferenced(page);
    
    list_del(&page->lru);
    ClearPageActive(page);
    list_add(&page->lru, &l_inactive);
    */
    return 0;
}

int __hate_vma_pages_range(struct vm_area_struct * vma, 
			   unsigned long start, unsigned long end,
			   enum hate_lvl intensity) {
  
    LIST_HEAD(l_inactive);
    unsigned long addr;
    struct page * page = NULL;
    
    DEBUG_PRINT("hate_vma_pages_range(): vma-range[s=%lu, e=%lu]; "	\
		"range[start=%lu, end=%lu]", vma->vm_start, vma->vm_end, 
		start, end);
    
    /* validate page ranges within vma. */
    BUG_ON(start < vma->vm_start || end > vma->vm_end);
    
    /* do not try to access the guard page of a stack vma */
    if (is_stack_guard_page(vma, start)) 
        start += PAGE_SIZE;
  
    for (addr = start; addr < end; addr += PAGE_SIZE) {
    
        page = __get_user_page(vma, addr);
      
	if (page != NULL) {
	  
	  if (PageLinked(page)) {
	      printk(KERN_EMERG "Cannot hate page in chain\n");
	  } else {
	    __hate_page(page, intensity);
	  }
      
      
	  DEBUG_PRINT("hate_vma_pages_range(): hated(address): %lu "	\
		      "in vma: %lu", start, (unsigned long)vma);
	} else {
	    DEBUG_PRINT("mlink_vma_pages_range(): no page found at %lu\n", 
			addr);
	}
    }
    
    return 0;
}

SYSCALL_DEFINE2(hate, unsigned long, start, size_t, len) {
  
    int r = -1;
    unsigned long end = start + len;
    unsigned long s = start;
    unsigned long e = end;
    
    struct mm_struct * mm = current->mm;
    struct vm_area_struct * vma = find_vma(mm, s); // retrieve first vma. 
    
    len = PAGE_ALIGN(len + (start & ~PAGE_MASK));
    start &= PAGE_MASK;
  
    /* check valid vma returned. */
    if (!vma || vma->vm_start >= end)
        return -1;
  
    DEBUG_PRINT("hate(): range[start=%lu, end=%lu]", start, end);
    
    for (s = start; s < end; s = e) {
      
        /* determine if current address lies outside range 
	   of current vma; if so move to next vma. */
        if (s >= vma->vm_end) 
	  vma = vma->vm_next;
	
	if (!vma || vma->vm_start >= end){
	    return -1;
	}
	
	/* determine current vma's end address. */
	e = end < vma->vm_end ? end : vma->vm_end;
      
	/* determine current vma's start address. */
	if (s < vma->vm_start){
	    s = vma->vm_start;
	}
	
	// TODO: allow user to select hate level. 
	r = __hate_vma_pages_range(vma, s, e, DISLIKE); 
    }
    
    return r;
}
		
