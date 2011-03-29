package mmu;

import java.util.Arrays;
import java.util.HashMap;

public class MemoryManager1 {
	
	public class Chain {
		
		public int id;
		private int capacity = Integer.MAX_VALUE;
		private int size = 0;
		
		private MemoryPage active = null;
		
		public Chain(int id) {
			this(id, Integer.MAX_VALUE);
		}
		
		public Chain(int id, int capacity) {
			this.id = id;
			this.capacity = capacity;
		}
		
		public void unchain() {
			for (MemoryPage p = this.active; p != null; ) {
				MemoryPage tmp = p;
				p = p.next;
				
				tmp.prev = null;
				tmp.next = null;
			}
		}
		
		public void unlink(MemoryPage page) {
			if (page != null) {
				 if (page.prev == null) { // head of list
					 this.active = page.next;
				 } else {
					 
					 MemoryPage previous = page.prev;
					 previous.next = page.next;
					 if (page.next != null) { // not tail of list
						 page.next.prev = previous;
					 }
				 }
				 this.size--;	 
			 }
		}
		
		public MemoryPage evict() {
			
			 long min = System.currentTimeMillis() + 1000;
			 MemoryPage page = null;
			 
			 for (MemoryPage p = this.active; p != null; p = p.next) {
				 if (p.time <= min) {
					 min = p.time;
					 page = p;		
				 }
			 }				
			 
			 this.unlink(page);
			 return page;
		}
		
		public void add(MemoryPage page) {
			if (this.size > this.capacity) {
				this.evict();
			}
			
			this.size++;
			
			if (this.active == null) {
				this.active = page;
			} else {
				this.active.prev = page;
				page.next = this.active;
				this.active = page;
			}
		}
		
		public long timestamp() {
			long timestamp = 0;
			
			for (MemoryPage p = this.active; p != null; p = p.next) {
				if (p.time > timestamp) {
					timestamp = p.time;
				}
			}
			
			return timestamp;
		}
	}
	
	private int link_id = 0;
	private HashMap<Integer, Chain> link_table = new HashMap<Integer, Chain>();
	
	private int size;
	
	public MemoryManager1(int size) {
		this.size = size;
	}
	
	public int love() {
		this.link_table.put(this.link_id, new Chain(this.link_id));
		return this.link_id++;
	}
	
	public int love(int capacity) {
		this.link_table.put(this.link_id, new Chain(capacity));
		return this.link_id++;
	}
	
	public void love(int page, int link) {
		MemoryPage p = this.memory.get(page);
		p.link = link;
		this.link_table.get(link).add(p);
		
		// this.directory.put(page, link); // TODO: 
	}
	
	public void hate(int link) {
		this.link_table.get(link).unchain();
	}
	
	public void hate(int page, int link) {
		this.link_table.get(link).unlink(this.memory.get(page));
	}
	
	private void evict() {
		
		HashMap<Integer, Long> timestamps = new HashMap<Integer, Long>();
		
		long min = System.currentTimeMillis() + 1000;
		Integer id = null;
		MemoryPage pg = null;
			
		for (MemoryPage p : this.memory.values()) {
			
			if (p.link >= 0 && !timestamps.containsKey(p.link)) {
				Chain c = this.link_table.get(p.link);
				long timestamp = c.timestamp();
				
				if (timestamp < min) {
					min = timestamp;
					id = p.link;
					pg = null;
				}
				
				timestamps.put(p.link, timestamp);
				
			} else if (p.link == -1 && p.time <= min) {
				min = p.time;
				pg = p;
				id = null;
			}
		}
		
		if (id != null) {
			pg = this.link_table.get(id).evict();
			pg.link = -1; // TODO: 
		}
		
		pg.evict();
		this.memory.remove(pg.address);
		
	}	

	public MemoryPage get(int address) {
		MemoryPage p = this.disk.get(address);
		if (p == null) {
			this.disk.put(address, p = new MemoryPage(address));
		}
		
		/*
		// should we re-establish link? 
		// requires bookkeeping for pid. 
		if (this.directory.containsKey(p.address)) {
			this.link_table.get(p.address).add(p);
		}
		*/
		return p;
	}
	
	public void access(int address) {
		
		MemoryPage page = this.memory.get(address);
		
		if (page == null) {
			if (this.memory.size() >= this.size) {
				this.evict();
			}
			
			page = this.get(address);
			page.load();
			page.ref();
			
			this.memory.put(address, page);
		} else {
			page.ref();
		}
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
	
	public void profileMemory() {
		for (MemoryPage block : this.memory.values()) {
			System.out.print(block.toString() + ';');
		}
	}
	
	//private HashMap<Integer, Integer> directory = new HashMap<Integer, Integer>();
	private HashMap<Integer, MemoryPage> memory = new HashMap<Integer, MemoryPage>();
	private HashMap<Integer, MemoryPage> disk = new HashMap<Integer, MemoryPage>();
	
}
