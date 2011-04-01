package mmu;

public class MultiLevelQueueReplacementPolicy implements IPageReplacementPolicy{
	
	private double loveQueueRatio;
	
	public MultiLevelQueueReplacementPolicy(){
		loveQueueRatio = 0.5;
	}
	
	public MemoryPage findPageToEvict(MemoryPage[] pages) {
		
		while(tooBigLoveQueue(pages)){
			reduceLoveQueue(pages);
		}
		
		
		long minTime = System.currentTimeMillis() + 1000;
		MemoryPage pageToEvict = null;
		
		for(MemoryPage page: pages){
			if(page.love == 0){
				if(page.time < minTime){
					pageToEvict = page;
				}
			}
		}
		
		return pageToEvict;
	}
	
	private boolean tooBigLoveQueue(MemoryPage[] pages){
		int loveCounter = 0;
		for(MemoryPage page: pages){
			if(page.love > 0){
				loveCounter++;
			}
		}
		
		return (loveCounter/pages.length) > loveQueueRatio;
	}
	
	private void reduceLoveQueue(MemoryPage[] pages){
		long minTime = System.currentTimeMillis() + 1000;
		MemoryPage pageToEvict = null;
		
		for(MemoryPage page: pages){
			if(page.love > 0){
				if(page.time < minTime){
					pageToEvict = page;
				}
			}
		}
		pageToEvict.love = 0;
	}

	
	public void hate(MemoryPage page) {
		page.love = 0;
		page.time = 0;
	}


	public void hate(MemoryPage page, int l) {
		page.love = 0;
		page.time = 0;
	}

	
	public void love(MemoryPage page, int l) {
		page.love = l;
	}

	
	public void love(MemoryPage page, int l, boolean pin) {
		page.love = l;
	}


	public String name() {
		return "LoveQueue";
	}

}
