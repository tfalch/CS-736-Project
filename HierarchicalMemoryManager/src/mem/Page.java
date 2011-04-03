package mem;

import util.Coordinator;

public class Page {

	private enum Counter {
		HIT,
		MISS,
		EVICT
	};
	
	int address = 0;
	long timestamp = 0;
	
	int resides = -1;   // residing memory chain
	int assigned = -1;  // assigned memory chain.
	Page next = null;
	Page prev = null;

	private int [] stats = {0,0,0};
	
	public Page(int address) {
		this.address = address;
	}
	
	public void load() {
		this.stats[Counter.MISS.ordinal()]++;
		this.timestamp = Coordinator.clock.generate();
	}
	
	public void ref() {
		this.stats[Counter.HIT.ordinal()]++;
		this.timestamp = Coordinator.clock.generate();
	}
	
	public void evict() {
		this.stats[Counter.EVICT.ordinal()]++;
	}
	
	public int refs() {
		return this.stats[Counter.HIT.ordinal()];
	}
	
	public int misses() {
		return this.stats[Counter.MISS.ordinal()];
	}
	
	public int evictions() {
		return this.stats[Counter.EVICT.ordinal()];
	}
	
	public String stats() {
		return "page::=[address=0x" + Integer.toHexString(address) + ";statistics:[r=" + 
		this.stats[Counter.HIT.ordinal()] +
		";m=" + this.stats[Counter.MISS.ordinal()] + ";e=" + 
		this.stats[Counter.EVICT.ordinal()] + "]";
	}
}
