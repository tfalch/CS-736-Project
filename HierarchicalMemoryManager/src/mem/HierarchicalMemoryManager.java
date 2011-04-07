package mem;

import java.text.DecimalFormat;
import java.util.Arrays;
import java.util.HashMap;

import mem.MemoryChain.EvictionPolicy;

import util.Coordinator;

public class HierarchicalMemoryManager implements IMemoryManager {

	private enum SystemQueue {
		HATED,
		SPILLOVER,
		DEFAULT
	};
	
	private int size = 0;
	
	private int chain_id = 0;
	private MemoryChain [] sys_chains = new MemoryChain[3];
	private HashMap<Integer, MemoryChain> usr_chains = new HashMap<Integer, MemoryChain>();
	
	private Memory memory = null; // physical memory.
	private PageTable pageTable = new PageTable(); // page table / virtual memory mapping.
	private HashMap<Integer, Page> swap = new HashMap<Integer, Page>(); // swap space.
	
	/**
	 * Creates and initializes a new instance.
	 * @param size memory capacity.
	 */
	public HierarchicalMemoryManager(int size) {
		this.size = size;
		
		for (SystemQueue q : SystemQueue.values()) {
			this.sys_chains[q.ordinal()] = new MemoryChain(this.chain_id++, EvictionPolicy.Random);
		}
		
		this.memory = new Memory(size);
	}
	
	private boolean is_sys_mem_chain(int chain) {
		return SystemQueue.HATED.ordinal() <= chain &&
		chain <= SystemQueue.DEFAULT.ordinal();
	}
	
	private boolean is_usr_mem_chain(int chain) {
		return !this.is_sys_mem_chain(chain);
	}
	
	/**
	 * Simulates page fault. 
	 * @param address virtual memory address.
	 * @return Page object.
	 */
	private Page load(int address) {
		
		Page p = this.swap.get(address);
		/* establish virtual memory mapping if page referenced for first time. */
		if (p == null) {
			p = new Page(address);
		}
		
		try {
			Thread.sleep(0); // simulate constant page fault time. 
		} catch (InterruptedException e) {
		}
		
		return p;
	}
	
	private void evict() {
		
		long minimum = Coordinator.clock.current() + 1;
		
		Page page = null;
		MemoryChain chain = null;
		
		if (this.sys_chains[SystemQueue.HATED.ordinal()].size() > 0) {
			chain = this.sys_chains[SystemQueue.HATED.ordinal()];
			page = chain.tail();
		} else if (this.sys_chains[SystemQueue.SPILLOVER.ordinal()].size() > 0) {
			chain = this.sys_chains[SystemQueue.SPILLOVER.ordinal()];
			for (Page p : chain) {
				if (p.timestamp < minimum) {
					page = p;
					minimum = p.timestamp;
				}
			}
		} else {
			
			for (Page p : this.sys_chains[SystemQueue.DEFAULT.ordinal()]) {
				if (p.timestamp < minimum) {
					page = p;
					minimum = p.timestamp;
					chain = this.sys_chains[SystemQueue.DEFAULT.ordinal()];
				}	
			}
			
			for (MemoryChain c : this.usr_chains.values()) {
				if (c.size() > 0 && c.timestamp() < minimum) {
					page = c.victim();
					minimum = c.timestamp();
					chain = c;
				}
			}
		}
		
		chain.unlink(page);
		this.memory.store(null, page.frame);
		this.pageTable.remove(page.address);
		this.swap.put(page.address, page); // store page to swap space.
	}
	
	/* (non-Javadoc)
	 * @see mem.IMemoryManager#capacity()
	 */
	@Override
	public int capacity() {
		return this.size;
	}
	
	/* (non-Javadoc)
	 * @see mem.IMemoryManager#access(int)
	 */
	@Override
	public void access(int address) {

		int frame = this.pageTable.lookup(address);  
		Page page = null;
		
		if (frame == -1) { /* page fault */
			
			frame = this.memory.free_frame();
			if (frame == -1) {
				this.evict();
				frame = this.memory.free_frame();
			}
			
			page = this.load(address);
			page.load();
			page.ref();
			
			this.memory.store(page, frame);
			this.pageTable.add(page.address, frame);
			
			/* re-establish externally broken links. */
			if (page.assigned != -1) {
				if (this.is_usr_mem_chain(page.assigned)) {
					this.usr_chains.get(page.assigned).link(page);
				} else {
					this.sys_chains[page.assigned].link(page);
				}
			} else {
				this.sys_chains[SystemQueue.DEFAULT.ordinal()].link(page);
			}
			
		} else {	
			page = this.memory.load(frame);
			page.ref();
		}
		
		if (this.is_usr_mem_chain(page.resides)) {
			this.usr_chains.get(page.resides).representative(page);
		}
	}

	/* (non-Javadoc)
	 * @see mem.IMemoryManager#createMemoryChain(EvictionPolicy)
	 */
	@Override
	public MemoryChain createMemoryChain(EvictionPolicy policy) {
		MemoryChain chain = new MemoryChain(this.chain_id++, policy);
		this.usr_chains.put(chain.id(), chain);
		
		return chain;
	}

	/* (non-Javadoc)
	 * @see mem.IMemoryManager#createMemoryChain(EvictionPolicy, int)
	 */
	@Override
	public MemoryChain createMemoryChain(EvictionPolicy policy, int capacity) {
		MemoryChain chain = new MemoryChain(this.chain_id++, policy, capacity);
		this.usr_chains.put(chain.id(), chain);
		
		return chain;
	}

	/* (non-Javadoc)
	 * @see mem.IMemoryManager#createMemoryChain(EvictionPolicy, int, boolean)
	 */
	@Override
	public MemoryChain createMemoryChain(EvictionPolicy policy, int capacity, boolean anchored) {
		throw new UnsupportedOperationException();
	}

	/* (non-Javadoc)
	 * @see mem.IMemoryManager#link(mem.MemoryChain, int)
	 */
	@Override
	public void link(MemoryChain chain, int address) {
		
		Page page = this.memory.load(this.pageTable.lookup(address));
		if (page.resides != -1) {
			if (this.is_sys_mem_chain(page.resides)) {
				this.sys_chains[page.resides].unlink(page);
			} else if (chain.id() != page.resides) {
				this.usr_chains.get(page.resides).unlink(page);
			}
		}
			
		page.assigned = chain.id();
		page = chain.link(page);
		if (page != null) {
			this.sys_chains[SystemQueue.SPILLOVER.ordinal()].link(page);
			page.assigned = SystemQueue.DEFAULT.ordinal(); /* re-assign page to default queue. */
		}
		
	}

	/* (non-Javadoc)
	 * @see mem.IMemoryManager#anchor(mem.MemoryChain, int)
	 */
	@Override
	public void anchor(MemoryChain chain, int address) {
		
		Page p = this.memory.load(this.pageTable.lookup(address));
		if (this.is_sys_mem_chain(p.resides)) {
			this.sys_chains[p.resides].unlink(p);
		} else if (p.resides != -1 && chain.id() != p.resides) {
			this.usr_chains.get(p.resides).unlink(p);
		}
		
		p.assigned = chain.id();
		p = chain.anchor(p);
		if (p != null) {
			this.sys_chains[SystemQueue.SPILLOVER.ordinal()].link(p);
			p.assigned = SystemQueue.DEFAULT.ordinal(); /* re-assign page to default queue. */
		}
	}

	/* (non-Javadoc)
	 * @see mem.IMemoryManager#unlink(int)
	 */
	@Override
	public void unlink(int address) {
		
		int frame = this.pageTable.lookup(address);
		if (frame != -1) {
			Page page = this.memory.load(frame);
			MemoryChain chain = this.is_sys_mem_chain(page.resides) ? this.sys_chains[page.resides] :
				this.usr_chains.get(page.resides);
			
			if (chain != this.sys_chains[SystemQueue.HATED.ordinal()]) {
				chain.unlink(page);
				page.assigned = SystemQueue.DEFAULT.ordinal();
				this.sys_chains[SystemQueue.HATED.ordinal()].link(page);
			}
		}
	}

	/* (non-Javadoc)
	 * @see mem.IMemoryManager#breakMemoryChain(mem.MemoryChain)
	 */
	@Override
	public void breakMemoryChain(MemoryChain chain) {
		Page [] pages = chain.unchain();
		for (Page page : pages) {
			page.assigned = SystemQueue.DEFAULT.ordinal();
			this.sys_chains[SystemQueue.HATED.ordinal()].link(page);	
		}
	}

	/* (non-Javadoc)
	 * @see mem.IMemoryManager#releaseMemoryChain(mem.MemoryChain)
	 */
	@Override
	public void releaseMemoryChain(MemoryChain chain) {
		this.breakMemoryChain(chain);
		this.usr_chains.remove(chain.id());
	}
	
	public void summary() {
		
		long hit = 0;
		long miss = 0;
		long evicted = 0;
		
		for (Page page : this.swap.values()) {
			hit += page.refs();
			miss += page.misses();
			evicted += page.evictions();
		}
		
		DecimalFormat df = new DecimalFormat("0.00");
		
		java.lang.System.out.println("statistics:[ref-count=" + hit + ";miss-count=" +
				miss + ";evicted-count=" + evicted + ";miss-ratio=" + 
				df.format((double)miss/hit) + "]");
		
	}
	
	public void stats() {
		Integer [] addresses = this.swap.keySet().toArray(new Integer[0]);
		
		Arrays.sort(addresses);
		for (int i : addresses) {
			Page p = this.swap.get(i);
			java.lang.System.out.println(p.stats());
		}
	}

}
