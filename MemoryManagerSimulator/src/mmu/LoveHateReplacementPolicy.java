package mmu;


public class LoveHateReplacementPolicy implements IPageReplacementPolicy {	
	
	private static int PIN_LEVEL = 255;
	
	public String name() {
		return "LHR";
	}
	
	public MemoryPage evict(MemoryPage [] pages) {
		
		long min = System.currentTimeMillis() + 1000;
		int love = Integer.MAX_VALUE;
		
		MemoryPage b = null;
		
		for (MemoryPage block : pages) {			
			
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
					if (block.love != PIN_LEVEL)
						block.love--;
					b = block;
			}
		}
	
		b.love = 0;	
		return b;
	}
	
	public void love(MemoryPage page, int level) {
		if (page != null) {
			page.love = Math.min(level, PIN_LEVEL);
		}
	}
	
	public void love(MemoryPage page, int level, boolean pin) {
		if (page != null) {
			page.love = pin ? PIN_LEVEL : Math.min(level, PIN_LEVEL - 1);
		}
	}
	
	public void hate(MemoryPage page, int level) {
		if (page != null) {
			page.love = level;
		}
	}
	
	public void hate(MemoryPage page) {
		if (page != null) {
			page.love = 0;
			page.time = 0;
		}
	}
}
