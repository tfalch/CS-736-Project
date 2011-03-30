package mmu;

public class MemoryPage {

	private enum Counters {
		HIT,
		MISS,
		EVICT
	};
	
	int address;
	int love;
	long time;
	
	int link = -1;
	MemoryPage next = null;
	MemoryPage prev = null;
	
	private int [] stats = {0,0,0,0};
	
	public MemoryPage(int address) {
		this.address = address;
	}
	
	public void load() {
		this.stats[Counters.MISS.ordinal()]++;
		this.time = System.currentTimeMillis();
	}
	
	public void ref() {
		this.stats[Counters.HIT.ordinal()]++;
		this.time = System.currentTimeMillis();
	}
	
	public void evict() {
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
