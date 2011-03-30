package mmu;

import java.util.Arrays;
import java.util.HashMap;

public class MemoryManager {

	private IPageReplacementPolicy policy;
	private int size;
	
	public MemoryManager(IPageReplacementPolicy policy, int size) {
		this.policy = policy;
		this.size = size;
	}
	
	private MemoryPage get(int address) {
		MemoryPage p = this.disk.get(address);
		if (p == null) {
			this.disk.put(address, p = new MemoryPage(address));
		}
		
		try {
			Thread.sleep(1);
		} catch (InterruptedException e) {
		}
		
		return p;
	}
	
	public void access(int address) {
		
		MemoryPage page = this.memory.get(address);
		
		if (page == null) {
			if (this.memory.size() >= this.size) {
				page = this.policy.evict(this.memory.values().toArray(new MemoryPage[0]));
				page.evict();
				this.memory.remove(page.address);
			}
			
			page = this.get(address);
			page.load();
			page.ref();
			
			this.memory.put(address, page);
		} else {
			page.ref();
		}
	}
	
	public void love(int page, int l) {
		this.policy.love(this.memory.get(page), l);
	}
	
	public void love(int page, int l, boolean pin) {
		this.policy.love(this.memory.get(page), l, pin);
	}
	
	public int capacity() {
		return this.size;
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
