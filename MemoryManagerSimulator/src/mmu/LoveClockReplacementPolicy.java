package mmu;

public class LoveClockReplacementPolicy implements IPageReplacementPolicy {

	private int clockHand;
	
	public LoveClockReplacementPolicy(){
		clockHand = 0;
	}
	
	public MemoryPage findPageToEvict(MemoryPage[] pages) {
		while(true){
			if(pages[clockHand].love == 0 && !pages[clockHand].referenced)
				return pages[clockHand];
			else{
				if(pages[clockHand].love == 0){
					pages[clockHand].referenced = false;
				}
				if(pages[clockHand].love > 0){
					pages[clockHand].love--;
					pages[clockHand].referenced = true;
				}
				clockHand = (clockHand + 1)%pages.length;
			}
		}
	}

	
	public void hate(MemoryPage page) {
		page.love = 0;
		page.referenced = false;
	}

	
	public void hate(MemoryPage page, int l) {
		hate(page);
	}

	
	public void love(MemoryPage page, int l) {
		page.love = l;
		page.referenced = true;
	}

	
	public void love(MemoryPage page, int l, boolean pin) {
		love(page, l);
	}

	
	public String name() {
		return "Love Clock";
	}

}
