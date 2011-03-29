package mmu;
import java.util.Arrays;
import java.util.HashMap;


public class LoveHateReplacementPolicy {	
	
	private int size;
	private HashMap<Integer, MemoryPage> memory;
	private HashMap<Integer, MemoryPage> pages = new HashMap<Integer, MemoryPage>(); // disk
	
	public LoveHateReplacementPolicy(int size) {
		this.size = size;
		this.memory = new HashMap<Integer, MemoryPage>(size);
	}
	
	private void evict() {
		
		long min = System.currentTimeMillis() + 1000;
		int love = Integer.MAX_VALUE;
		
		MemoryPage b = null;
		
		for (MemoryPage block : this.memory.values()) {			
			
			// remove block with minimum love & time value
			if (block.love == 0) {

				if (love > 0) { 
					love = 0;
					min = block.time;
					b = block;
				} else if (block.time < min) {
					love = 0; // superfluous.
					min = block.time;
					b = block;
				}
			} else if (block.love < love ||
					(block.love == love && block.time < min)) {
				
					min = block.time;
					love = block.love;
					block.love--;
					b = block;
			}
		}
		
		this.memory.remove(b.address);
		
		b.evict();
		b.love = 0;
	}
	
	public int capacity() {
		return this.size;
	}
	
	private MemoryPage get(int page) {
		
		MemoryPage b = this.pages.get(page);
		if (b == null) {
			this.pages.put(page, b = new MemoryPage(page));
		}
		
		return b;
	}
	
	public void access(int page) {
		MemoryPage block = this.memory.get(page);
		
		if (block == null) {
			if (this.memory.size() >= this.size) {
				this.evict();
			}
			
			block = this.get(page);
			block.load();
			block.ref();
			
			this.memory.put(page, block);
		} else {
			block.ref();
		}
	}
	
	public void love(int page, int level) {
		MemoryPage block = this.memory.get(page);
		
		if (block != null) {
			block.love = level;
		}
	}
	
	public void hate(int page) {
		MemoryPage block = this.memory.get(page);
		
		if (block != null) {
			block.love = 0;
			block.time = 0;
		}
	}
	
	public void profileMemory() {
		for (MemoryPage block : this.memory.values()) {
			System.out.print(block.toString() + ';');
		}
	}
	
	public void stats() {
		
		Integer [] addresses = this.pages.keySet().toArray(new Integer[0]);
		
		Arrays.sort(addresses);
		for (int i : addresses) {
			MemoryPage p = this.pages.get(i);
			System.out.println(p.stats());
		}
	}
}
