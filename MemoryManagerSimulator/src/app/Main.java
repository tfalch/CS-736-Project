package app;
import java.awt.Point;
import java.io.IOException;
import java.util.Collection;
import java.util.HashMap;

import mmu.ChainedLeastRecentlyUsedReplacementPolicy;
import mmu.IPageReplacementPolicy;
import mmu.LRUReplacementPolicy;
import mmu.LoveHateReplacementPolicy;
import mmu.MemoryManager;

public class Main {	
	
	private static void simulate_block_join(IPageReplacementPolicy p, int l) {

		long start = System.currentTimeMillis();
		MemoryManager m = new MemoryManager(p, 30);
		new AccessPatterns(m).trace_block_join(l, 125, 500);
		long end = System.currentTimeMillis();
		
		System.out.print(p.name() + ": ");
		System.out.print("duration=" + (end - start) + ";"); m.summary();
	}
	
	private static void simulate_index_join(IPageReplacementPolicy p, int l) {

		long start = System.currentTimeMillis();
		MemoryManager m = new MemoryManager(p, 15);
		new AccessPatterns(m).trace_index_join(l, 2500, 100000000, 64);
		long end = System.currentTimeMillis();
		
		System.out.print(p.name() + ": ");
		System.out.print("duration=" + (end - start) + ";"); m.summary();
	}
	
	public static void run() {
		
		System.out.println("\t\t===================================================");
		System.out.println("\t\t\t   Simulating Nested Block Join");
		System.out.println("\t\t===================================================");		
		
		ChainedLeastRecentlyUsedReplacementPolicy clru = new ChainedLeastRecentlyUsedReplacementPolicy();
		simulate_block_join(clru, clru.love().id());
		simulate_block_join(new LoveHateReplacementPolicy(), 254);
		simulate_block_join(new LRUReplacementPolicy(), -1);
		
		System.out.println();
		
		System.out.println("\t\t===================================================");
		System.out.println("\t\t\t       Simulating Index Join");
		System.out.println("\t\t===================================================");
		
		clru = new ChainedLeastRecentlyUsedReplacementPolicy();
		simulate_index_join(clru, clru.love(true).id());
		simulate_index_join(new LoveHateReplacementPolicy(), 254);
		simulate_index_join(new LRUReplacementPolicy(), -1);
		System.out.println();
	}
	
	public static void main(String [] args) throws IOException {			
		Main.run();
	}
}
