#ifndef MCHAIN_H_
#define MCHAIN_H_

#include <sys/syscall.h>
#include <linux/unistd.h>

/* wrappers for system calls at user-level. */

#define NR_sys_new_mem_chain      341
#define NR_sys_set_mem_chain_attr 342
#define NR_sys_link_addr_rng      343
#define NR_sys_anchor             344
#define NR_sys_unlink_addr_rng    345
#define NR_sys_brk_mem_chain      346
#define NR_sys_rls_mem_chain      347
#define NR_sys_hate               348  

#define mchain() syscall(NR_sys_new_mem_chain)
#define set_mchain_attr(cid, addr) syscall(NR_sys_set_mem_chain_attr, \
					   cid, (addr))
#define mlink(cid, start, len) syscall(NR_sys_link_addr_rng, cid, (start), len)
#define anchor(cid, addr) syscall(NR_sys_anchor, cid, (addr))
#define munlink(start, len) syscall(NR_sys_unlink_addr_rng, (start), len)
#define brk_mchain(cid) syscall(NR_sys_brk_mem_chain, cid)
#define rls_mchain(cid) syscall(NR_sys_rls_mem_chain, cid)
#define hate(start, len) syscall(NR_sys_hate, start, len)

#define mchain_strerror(errno) chain_errors[errno]

static const char * chain_errors[] = {
  "Success",
  "Invalid chain descriptor",
  "Too many open chains",
  "Invalid address range"
};

#endif /* MCHAIN_H_ */
