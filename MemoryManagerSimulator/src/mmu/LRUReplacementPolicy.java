package mmu;

public class LRUReplacementPolicy implements IPageReplacementPolicy {

	public String name() {
		return "LRU";
	}
	
	@Override
	public MemoryPage findPageToEvict(MemoryPage[] pages) {
		long min = System.currentTimeMillis() + 1000;
		MemoryPage evicted = null;
		
		for (MemoryPage memoryPage : pages) {
			if (memoryPage.time < min) {
				evicted = memoryPage;
				min = memoryPage.time;
			}
		}
		
		return evicted;
	}

	@Override
	public void love(MemoryPage page, int l) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void love(MemoryPage page, int l, boolean pin) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void hate(MemoryPage page) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void hate(MemoryPage page, int l) {
		// TODO Auto-generated method stub
		
	}

}
