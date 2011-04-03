package mem;

import mem.MemoryChain.EvictionPolicy;

public interface IMemoryManager {	
	
	/**
	 * Accesses the page associated with the given address. If the page 
	 * is not found in physical memory, a page fault occurs and the page 
	 * is retrieved.
	 * @param address address of page to access.
	 */
	public void access(int address);
	
	/**
	 * Gets the memory's capacity.
	 * @return page capacity of the physical memory.
	 */
	public int capacity();
	
	/**
	 * Creates and initializes a new user maintained memory chain.
	 * @param local eviction policy to be used.
	 * @return memory chain object.
	 */
	public MemoryChain createMemoryChain(EvictionPolicy policy);
	
	/**
	 * Creates a new instance of an unbounded memory chain.
	 * @param local eviction policy to be used.
	 * @param capacity max links in memory chain.
	 * @return memory chain object.
	 */
	public MemoryChain createMemoryChain(EvictionPolicy policy, int capacity);
	
	/**
	 * Creates a new instance of a bounded memory chain object.
	 * @param local eviction policy to be used.
	 * @param capacity max links in memory chain.
	 * @param anchored flag indicating if the memory chain is anchored or not.
	 * @return memory chain object.
	 */
	public MemoryChain createMemoryChain(EvictionPolicy policy, int capacity, boolean anchored);
	
	/**
	 * Adds a memory page to the specified memory chain. Previously established 
	 * links involving the chain are automatically broken.
	 * @param chain memory chain to associate page with.
	 * @param address page's address.
	 */
	public void link(MemoryChain chain, int address);
	
	/**
	 * Anchors a page in the specified memory chain. Previously established 
	 * links involving the chain are automatically broken. Furthermore, chain's
	 * current anchor chain is automatically 'raised'. 
	 * @param chain memory chain object to associate page with.
	 * @param address page's address.
	 */
	public void anchor(MemoryChain chain, int address);
	
	/**
	 * Removes the page from any previously established links, and moves the page
	 * to the Hated Queue.
	 * @param address page's address.
	 */
	public void unlink(int address);
	
	/**
	 * Moves all pages associated with the memory chain to the Hated Queue.  
	 * @param chain memory chain object.
	 */
	public void breakMemoryChain(MemoryChain chain);
	
	/**
	 * Breaks the memory chain as well as release any resources held by the chain.
	 * @param chain memory chain object. 
	 */
	public void releaseMemoryChain(MemoryChain chain);
}
