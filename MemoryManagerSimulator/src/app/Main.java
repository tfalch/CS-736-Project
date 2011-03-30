package app;
import java.io.IOException;

import mmu.ChainedLeastRecentlyUsedReplacementPolicy.Chain;
import mmu.IPageReplacementPolicy;
import mmu.LRUReplacementPolicy;
import mmu.LoveHateReplacementPolicy;
import mmu.MemoryManager;
import mmu.ChainedLeastRecentlyUsedReplacementPolicy;

public class Main {	
	
	public static void simulate_block_join(IPageReplacementPolicy p, int l) {
		
		long start = System.currentTimeMillis();
		MemoryManager m = new MemoryManager(p, 15);
		new AccessPatterns(m).trace_block_join(l);
		long end = System.currentTimeMillis();
		
		System.out.print(p.name() + ": ");
		System.out.print("duration=" + (end - start) + ";"); m.summary();
	}
	
	public static void simulate_index_join(IPageReplacementPolicy p, int l) {
		long start = System.currentTimeMillis();
		MemoryManager m = new MemoryManager(p, 3);
		new AccessPatterns(m).trace_index_join(l);
		long end = System.currentTimeMillis();
		
		System.out.print(p.name() + ": ");
		System.out.print("duration=" + (end - start) + ";"); m.summary();
	}
	
	public static void main(String [] args) throws IOException {				
		
		System.out.println("\t\t===================================================");
		System.out.println("\t\t\t   Simulating Nested Block Join");
		System.out.println("\t\t===================================================");		
		
		simulate_block_join(new LRUReplacementPolicy(), -1);
		simulate_block_join(new LoveHateReplacementPolicy(), 20);
		ChainedLeastRecentlyUsedReplacementPolicy clru = new ChainedLeastRecentlyUsedReplacementPolicy();
		simulate_block_join(clru, clru.love().id());
		System.out.println();
		
		System.out.println("\t\t===================================================");
		System.out.println("\t\t\t       Simulating Index Join");
		System.out.println("\t\t===================================================");
		
		simulate_index_join(new LRUReplacementPolicy(), -1);
		simulate_index_join(new LoveHateReplacementPolicy(), 10);
		clru = new ChainedLeastRecentlyUsedReplacementPolicy();
		simulate_index_join(clru, clru.love(true).id());
		System.out.println();
	}
}
