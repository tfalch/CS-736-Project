package mmu;

public interface IMemoryManager {

	public void access(int address);
	public void love(int page, int l);
	public void love(int page, int l, boolean pin);
	public void hate(int page, int l);
	public int capacity();
	
	public void summary();
}
