package mmu;

import java.text.DecimalFormat;
import java.util.Arrays;
import java.util.HashMap;

public class MemoryManager {

	private IPageReplacementPolicy policy;
	private int size;
	
	public MemoryManager(IPageReplacementPolicy policy, int size) {
		this.policy = policy;
		this.size = size;
	}
	
	private MemoryPage getFromDisk(int address) {
		MemoryPage p = this.disk.get(address);
		if (p == null) {
			this.disk.put(address, p = new MemoryPage(address));
		}
		
		//TODO is this needed?
		try {
			Thread.sleep(0);
		} catch (InterruptedException e) {
		}
		
		return p;
	}
	
	public void access(int address) {
		
		MemoryPage page = this.memory.get(address);
		
		//If not in memory, get if from disk
		if (page == null) {
			
			//If memory is full, evict a page
			if (this.memory.size() >= this.size) {
				page = this.policy.evict(this.memory.values().toArray(new MemoryPage[0]));
				page.updateEvictStats();
				this.memory.remove(page.address);
			}
			
			
			page = this.getFromDisk(address);
			page.updateLoadStats();
			
			//TODO should this be here?
			page.updateRefStats();
			
			this.memory.put(address, page);
		} else {
			page.updateRefStats();
		}
	}
	
	public void love(int page, int l) {
		this.policy.love(this.memory.get(page), l);
	}
	
	public void love(int page, int l, boolean pin) {
		this.policy.love(this.memory.get(page), l, pin);
	}
	
	public void hate(int page, int l) {
		this.policy.hate(this.memory.get(page), l);
	}
	
	public int capacity() {
		return this.size;
	}
	
	public void summary() {
		
		long hit = 0;
		long miss = 0;
		long evicted = 0;
		
		for (MemoryPage page : this.disk.values()) {
			hit += page.hits();
			miss += page.misses();
			evicted += page.evictions();
		}
		
		DecimalFormat df = new DecimalFormat("0.00");
		
		System.out.println("page statistics:[hit-count=" + hit + ";miss-count=" +
				miss + ";evicted-count=" + evicted + "; miss-ratio=" + 
				df.format((double)miss/hit) + "]");
		
	}
	
	public void stats() {
		Integer [] addresses = this.disk.keySet().toArray(new Integer[0]);
		
		Arrays.sort(addresses);
		for (int i : addresses) {
			MemoryPage p = this.disk.get(i);
			System.out.println(p.stats());
		}
	}
	
	public void profile() {
		for (MemoryPage block : this.memory.values()) {
			System.out.print(block.toString() + ';');
		}
	}
	
	//private HashMap<Integer, Integer> directory = new HashMap<Integer, Integer>();
	private HashMap<Integer, MemoryPage> memory = new HashMap<Integer, MemoryPage>();
	private HashMap<Integer, MemoryPage> disk = new HashMap<Integer, MemoryPage>();
	
}
