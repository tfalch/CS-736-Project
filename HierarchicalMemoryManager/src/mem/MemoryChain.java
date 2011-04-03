package mem;

import java.util.Iterator;

import util.Coordinator;

public class MemoryChain implements Iterable<Page> {

	private int id;
	private int size = 0;
	private int capacity = Integer.MAX_VALUE;
	private boolean bounded = false;
	private boolean anchored = false; // update last access time on eviction of a node.
	
	private Page anchor = null; // last node to be evicted from chain
	private Page head = null;
	private Page tail = null;
	
	Page victim = null;
	private long timestamp = 0; // chain's timestamp.
	private int sample = 0; // timestamp's sample #. 
	
	/**
	 * Creates a new instance of an unbounded memory chain.
	 * @param id chain's id.
	 */
	public MemoryChain(int id) {
		this(id, Integer.MAX_VALUE, false, false);
	}
	
	/**
	 * Creates a new instance of an unbounded memory chain
	 * @param id chain's id.
	 * @param anchored flag indicating if the memory chain is anchored or not.
	 */
	public MemoryChain(int id, boolean anchored) {
		this(id, Integer.MAX_VALUE, false, false);
	}
	
	/**
	 * Creates a new instance of a bounded memory chain.
	 * @param id chain's id.
	 * @param capacity maximum number of pages that may reside in memory chain.
	 */
	public MemoryChain(int id, int capacity) {
		this(id, capacity, true, false);
	}
	
	/**
	 * Creates a new instance of a bounded memory chain.
	 * @param id chain's id.
	 * @param capacity maximum number of pages that may reside in memory chain.
	 * @param anchored flag indicating if the memory chain is anchored or not.
	 */
	public MemoryChain(int id, int capacity, boolean anchored) {
		this(id, capacity, true, anchored);
	}	
	
	/**
	 * Creates and initializes a new instance of a memory chain.
	 * @param id chain's id.
	 * @param capacity maximum number of pages that may reside in memory chain.
	 * @param bounded flag indicating if the chain is bounded.
	 * @param anchored flag indicating if the memory chain is anchored or not.
	 */
	private MemoryChain(int id, int capacity, boolean bounded, boolean anchored) {
		this.id = id;
		this.capacity = capacity;
		this.bounded = false;
		this.anchored = anchored;
	}
	
	/**
	 * Gets the chain's id.
	 * @return chain's id.
	 */
	public int id() {
		return this.id;
	}
	
	/**
	 * Sets whether the chain's time stamp is updated on removal of a link. 
	 * @param flag indicates if anchoring is enabled or not.
	 */
	public void anchored(boolean flag) {
		this.anchored = flag;
	}
	
	/**
	 * Gets the anchoring status of the chain
	 * @return anchoring status
	 */
	public boolean anchored() {
		return this.anchored;
	}
	
	/**
	 * Gets the number of pages linked to the memory chain.
	 * @return number of linked pages.
	 */
	int size() {
		return this.size;
	}
	
	private void reset(Page page) {
		page.prev = null;
		page.next = null;
		page.resides = -1;
	}
	
	Page head() {
		return this.head;
	}
	
	Page tail() {
		return this.tail;
	}
	
	/**
	 * Removes all linked pages from the memory chain.
	 * @return Array containing previously linked pages.
	 */
	Page [] unchain() {
		
		Page [] pages = new Page[this.size];
		int i = 0;
		
		for (Page p = this.head; p != null; ) {
			Page tmp = p;
			p = p.next;
			
			this.reset(tmp);
			pages[i++] = tmp;
		}
		
		this.head = null;
		this.tail = null;
		this.anchor = null;
		this.size = 0;
		
		return pages;
	}
	
	/**
	 * Removes a page from the memory chain.
	 * @param page Page object to be removed.
	 */
	void unlink(Page page) {
		if (page != null && page.resides != -1) {
			 if (page.prev == null) { // head of list
				 this.head = page.next;
				 if (this.head != null) {
					 this.head.prev = null;
				 }
			 } else {
				 
				 Page previous = page.prev;
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
			 
			 this.reset(page);
			 
			 this.size--;	 
		}
	}
	
	/**
	 * Evicts a page from the memory chain.
	 * @return victim page.
	 */
	Page evict() { 
		
		 long min = Coordinator.clock.current() + 1; 
		 Page page = null;
		 
		 // force eviction of non-anchor node or anchor node if no other
		 // viable node found.
		 if (this.size == 1) {
			 page = this.head;
		 } else if (this.anchor != null) {
			 page = this.anchor.prev != null ? this.anchor.prev : this.anchor.next;
		 }
		
		 for (Page h = this.head, t = this.tail; h != null && h.prev != t; h = h.next, t = t.prev) {
			 
			 if (h.timestamp <= min && h != this.anchor) {
				 min = h.timestamp;
				 page = h;		
			 }
			 
			 if (t.timestamp <= min && t != this.anchor) {
				 min = t.timestamp;
				 page = t;		
			 }
		 }
		 
		 if (this.anchored && this.head != null) {
			 if (this.anchor != null) {
				 this.anchor.timestamp = Coordinator.clock.generate();
			 } else {
				 this.head.timestamp = Coordinator.clock.generate();
			 }
		 }
		 
		 this.unlink(page);
		 page.timestamp = min; 
		 
		 return page;
	}
	
	/**
	 * Adds a page object to the memory chain. If the memory chain is bounded and
	 * the addition of the new page results in an overflow, a page is selected for 
	 * eviction and removed from the chain.
	 * @param page Page to be added.
	 * @return Victim page as a result of possible overflow. 
	 */
	Page link(Page page) {
		
		Page evicted = null;
		
		// check for duplicates.
		if (page.resides == this.id) {
			return null;
		}
		
		if (this.size >= this.capacity && this.bounded) {
			evicted = this.evict();
			this.unlink(evicted); 
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
		
		page.resides = this.id;
		
		return evicted;
	}
	
	/**
	 * Adds and anchors the page in the memory chain.
	 * @param Page object to be anchored. 
	 * @return Victim page if page addition results in overflow.
	 */
	Page anchor(Page page) {
		Page p = this.link(page);
		this.anchor = page;
		
		return p;
	}
	
	/**
	 * Gets the effective time stamp of the memory chain.  
	 * @param sample non-decreasing sequence number.
	 * @return Chain's effective time stamp.
	 */
	long timestamp(int sample) {
		
		long minimum = Coordinator.clock.current() + 1;
		
		if (this.sample == sample) {
			return this.timestamp;
		}
		
		this.victim = null;
		
		for (Page h = this.head, t = this.tail; h != null && h.prev != t; h = h.next, t = t.prev) {
		
			if (h.timestamp > this.timestamp) {
				this.timestamp = h.timestamp;
			} else if (h.timestamp < minimum) {
				minimum = h.timestamp;
				this.victim = h;
			}
			
			if (t.timestamp > this.timestamp) {
				this.timestamp = t.timestamp;
			} else if (t.timestamp < minimum) {
				minimum = t.timestamp;
				this.victim = t;
			} 
		}
		
		this.sample = sample;
		return this.timestamp;
	}

	@Override
	public Iterator<Page> iterator() {
		return new MemoryChainIterator(this.head);
	}
	
	class MemoryChainIterator implements Iterator<Page> {

		private Page page = null;
		
		public MemoryChainIterator(Page head) {
			this.page = head;
		}
		
		@Override
		public boolean hasNext() {
			return this.page != null;
		}

		@Override
		public Page next() {
			Page p = page;
			this.page = this.page.next;
			
			return p;
		}

		@Override
		public void remove() {
			throw new UnsupportedOperationException();
		}
		
	}
}
