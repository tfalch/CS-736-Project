package mmu.policy;

import java.util.HashMap;

import util.Coordinator;

import mmu.MemoryPage;

public class ChainedLeastRecentlyUsedReplacementPolicy implements IPageReplacementPolicy {
	
	public class Chain {
		
		public int id;
		private int capacity = Integer.MAX_VALUE;
		private int size = 0;
		private boolean anchored = false; // update last access time on eviction of a node.
		
		private MemoryPage anchor = null; // last node to be evicted from chain
		private MemoryPage head = null;
		private MemoryPage tail = null;
		
		private long timestamp = 0; // chain's timestamp.
		private int sample = 0; // timestamp's sample #. 
		
		private Chain(int id) {
			this(id, false);
		}
		
		private Chain(int id, boolean anchored) {
			this(id, Integer.MAX_VALUE, anchored);
		}
		
		private Chain(int id, int capacity) {
			this(id, Integer.MAX_VALUE, false);
		}
		
		private Chain(int id, int capacity, boolean anchored) {
			this.id = id;
			this.capacity = capacity;
			this.anchored = anchored;
		}	
		
		public int id() {
			return this.id;
		}
		
		public int size() {
			return this.size;
		}
		
		/**
		 * set whether the chain's time stamp is updated on removal of a link. 
		 * @param flag indicates if anchoring is enabled or not.
		 */
		public void anchored(boolean flag) {
			this.anchored = flag;
		}
		
		/**
		 * gets the anchoring status of the chain
		 * @return anchoring status
		 */
		public boolean anchored() {
			return this.anchored;
		}
		
		private void remove_link(MemoryPage page) {
			page.prev = null;
			page.next = null;
			page.link = -1;
		}
		
		private void unchain() {
			for (MemoryPage p = this.head; p != null; ) {
				MemoryPage tmp = p;
				p = p.next;
				
				this.remove_link(tmp);
			}
						
			this.head = null;
			this.tail = null;
			this.anchor = null;
			this.size = 0;
		}
		
		private void unlink(MemoryPage page) {
			if (page != null && page.link != -1) {
				 if (page.prev == null) { // head of list
					 this.head = page.next;
					 if (this.head != null) {
						 this.head.prev = null;
					 }
				 } else {
					 
					 MemoryPage previous = page.prev;
					 previous.next = page.next;
					 if (page.next != null) { // not tail of list
						 page.next.prev = previous;
					 } else { // update tail.
						 this.tail = page.prev;
					 }
				 }
				 
				 if (this.anchor == page) {
					 this.anchor = null;
				 }
				 
				 this.remove_link(page);
				 
				 this.size--;	 
			}
		}
		
		public MemoryPage evict() {
			
			 long min = Coordinator.currentSequence() + 1; 
			 MemoryPage page = null;
			 
			 // force eviction of non-anchor node or anchor node if no other
			 // viable node found.
			 if (this.size == 1) {
				 page = this.head;
			 } else if (this.anchor != null) {
				 page = this.anchor.prev != null ? this.anchor.prev : this.anchor.next;
			 }
			
			 for (MemoryPage p = this.head; p != null; p = p.next) {
				 if (p.time <= min && p != this.anchor) {
					 min = p.time;
					 page = p;		
				 }
			 }
			 
			 if (this.anchored && this.head != null) {
				 this.head.time = Coordinator.nextSequence();
			 }
			 
			 this.unlink(page);
			 page.time = min; 
			 
			 return page;
		}
		
		private boolean add(MemoryPage page) {
			
			// check for duplicates.
			if (page.link == this.id) {
				return true;
			}

			if (this.size >= this.capacity) {
				this.evict();
			}
			
			this.size++;
			
			if (this.head == null) {
				this.head = page;
				this.tail = page;
			} else {
				this.head.prev = page;
				page.next = this.head;
				this.head = page;
			}
			
			page.link = this.id;
			
			return true;
		}
		
		private boolean anchor(MemoryPage page) {
			if (this.add(page)) {
				this.anchor = page;
				return true;
			}
			
			return false;
		}
		
		public long timestamp(int sample) {
			
			if (this.sample == sample) {
				return this.timestamp;
			}
			
			for (MemoryPage p = this.head; p != null; p = p.next) {
				if (p.time > this.timestamp) {
					this.timestamp = p.time;
				}
			}
			
			this.sample = sample;
			return this.timestamp;
		}
	}
	
	private int link_id = 0;
	private HashMap<Integer, Chain> link_table = new HashMap<Integer, Chain>();
	
	public String name() {
		return "C-LRU";
	}
	
	/**
	 * creates a new chain
	 * @return chain object.
	 */
	public Chain love() {
		Chain c = new Chain(this.link_id++);
		this.link_table.put(c.id, c);
		return c;
	}
	
	/**
	 * create a new chain
	 * @param anchored flag indicating if chain is anchored.
	 * @return chain.
	 */
	public Chain love(boolean anchored) {
		Chain c = new Chain(this.link_id++, anchored);
		this.link_table.put(c.id, c);
		return c;
	}
	
	/**
	 * creates a new chain
	 * @param capacity maximum # of links. 
	 * @return chain.
	 */
	public Chain love(int capacity) {
		Chain c = new Chain(this.link_id++, capacity);
		this.link_table.put(c.id, c);
		return c;
	}
	
	/**
	 * creates a new chain
	 * @param capacity maximum # of links.
	 * @param anchored flag indicating if chain is anchored.
	 * @return chain.
	 */
	public Chain love(int capacity, boolean anchored) {
		Chain c = new Chain(this.link_id++, capacity, true);
		this.link_table.put(c.id, c);
		return c;
	}
	
	public void love(MemoryPage page, int link) {
		if (page.link != -1 && page.link != link) {
			this.link_table.get(page.link).unlink(page);
		}
		this.link_table.get(link).add(page);
	}
	
	public void love(MemoryPage page, int link, boolean pin) {
		if (page.link != -1 && page.link != link) {
			this.link_table.get(page.link).unlink(page);
		}
		
		if (pin) {
			this.link_table.get(link).anchor(page);
		} else {
			this.love(page, link);
		}
	}
	
	public void hate(MemoryPage page) {
		if (page.link != -1) {
			this.link_table.get(page.link).unchain();
		}
	}
	
	public void hate(MemoryPage page, int link) {
		this.link_table.get(link).unlink(page);
	}
	
	public void hate(int link) {
		this.link_table.get(link).unchain();
		this.link_table.remove(link);
	}
	
	private int sample = 0;
	public MemoryPage findPageToEvict(MemoryPage [] pages) {
		
		long min = Coordinator.currentSequence() + 1;
		Integer id = null;
		MemoryPage pg = null;
			
		this.sample++;
		for (MemoryPage p : pages) {

			if (p.link >= 0) {
				Chain c = this.link_table.get(p.link);
				long timestamp = c.timestamp(this.sample);
				
				if (timestamp < min) {
					min = timestamp;
					id = p.link;
					pg = null;
				}
				
			} else if (p.link == -1 && p.time <= min) {
				min = p.time;
				pg = p;
				id = null; 
			}
		}
		
		if (id != null) {
			pg = this.link_table.get(id).evict();
			pg.link = -1; 
		}
		
		return pg;
	}	
	
}
