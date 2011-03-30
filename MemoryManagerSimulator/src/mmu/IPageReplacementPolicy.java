package mmu;

public interface IPageReplacementPolicy {

	public MemoryPage evict(MemoryPage [] pages);
	
	public void love(MemoryPage page, int l);
	public void love(MemoryPage page, int l, boolean pin);
	public void hate(MemoryPage page);
	public void hate(MemoryPage page, int l);
}