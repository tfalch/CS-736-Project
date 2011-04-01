package mmu;

public class ClockReplacementPolicy implements IPageReplacementPolicy {

	private int clockHand;
	
	public ClockReplacementPolicy(){
		clockHand = 0;
	}
	
	public MemoryPage findPageToEvict(MemoryPage[] pages) {
		while(true){
			if(!pages[clockHand].referenced)
				return pages[clockHand];
			else{
				pages[clockHand].referenced = false;
				clockHand = (clockHand + 1)%pages.length;
			}
		}
	}

	@Override
	public void hate(MemoryPage page) {
		// TODO Auto-generated method stub

	}

	@Override
	public void hate(MemoryPage page, int l) {
		// TODO Auto-generated method stub

	}

	@Override
	public void love(MemoryPage page, int l) {
		// TODO Auto-generated method stub

	}

	@Override
	public void love(MemoryPage page, int l, boolean pin) {
		// TODO Auto-generated method stub

	}

	public String name() {
		return "Clock";
	}

}
