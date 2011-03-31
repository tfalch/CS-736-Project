package mmu;

public interface IPageReplacementPolicy {

	public String name();
	
	public MemoryPage findPageToEvict(MemoryPage [] pages);
	
	public void love(MemoryPage page, int l);
	public void love(MemoryPage page, int l, boolean pin);
	public void hate(MemoryPage page);
	public void hate(MemoryPage page, int l);
}
