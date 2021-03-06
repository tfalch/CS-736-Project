package mmu;

import util.Coordinator;

public class MemoryPage {

	private enum Counters {
		HIT,
		MISS,
		EVICT
	};
	
	int address;
	public int love;
	public long time;
	public boolean referenced;
	
	public int link = -1;
	public MemoryPage next = null;
	public MemoryPage prev = null;
	
	private int [] stats = {0,0,0,0};
	
	public MemoryPage(int address) {
		this.address = address;
	}
	
	public void updateLoadStats() {
		this.stats[Counters.MISS.ordinal()]++;
		this.time = Coordinator.nextSequence();
	}
	
	public void updateRefStats() {
		stats[Counters.HIT.ordinal()]++;
		time = Coordinator.nextSequence();
		referenced = true;
	}
	
	public void updateEvictStats() {
		this.stats[Counters.EVICT.ordinal()]++;
	} 
	
	public int hits() {
		return this.stats[Counters.HIT.ordinal()];
	}
	
	public int misses() {
		return this.stats[Counters.MISS.ordinal()];
	}
	
	public int evictions() {
		return this.stats[Counters.EVICT.ordinal()];
	}
	
	public String toString() {
		return "0x" + Integer.toHexString(this.address);
	}
	
	public String stats() {
		return "page::=[address=0x" + Integer.toHexString(address) + ";love=" + love + ";statistics:[h=" + 
		this.stats[Counters.HIT.ordinal()] +
		";m=" + this.stats[Counters.MISS.ordinal()] + ";e=" + 
		this.stats[Counters.EVICT.ordinal()] + "]";
	}
}
