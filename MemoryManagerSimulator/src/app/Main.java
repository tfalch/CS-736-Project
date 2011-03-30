package app;
import java.io.IOException;

import mmu.ChainedLeastRecentlyUsedReplacementPolicy.Chain;
import mmu.IPageReplacementPolicy;
import mmu.LRUReplacementPolicy;
import mmu.LoveHateReplacementPolicy;
import mmu.MemoryManager;
import mmu.ChainedLeastRecentlyUsedReplacementPolicy;

public class Main {
	
	public static void main(String [] args) throws IOException {
		
		int size = 15;
		
		System.out.println("LHR on Nested Block Join");
		long start = System.currentTimeMillis();
		
		IPageReplacementPolicy policy = new LoveHateReplacementPolicy();
		MemoryManager m = new MemoryManager(policy, size);
		new AccessPatterns(m).trace_block_join(15);
		long end = System.currentTimeMillis();
		m.stats();
		System.out.println("duration = " + (end - start));
		System.out.println("===================================================");
		System.in.read();
		
		
		System.out.println("C-LRU on Nested Block Join");
		start = System.currentTimeMillis();
		policy = new ChainedLeastRecentlyUsedReplacementPolicy();
		m = new MemoryManager(policy, size);
		Chain c = ((ChainedLeastRecentlyUsedReplacementPolicy)policy).love(true);
		new AccessPatterns(m).trace_block_join(c.id());	
		end = System.currentTimeMillis();
		
		m.stats();
		System.out.println("duration = " + (end - start));
		System.out.println("===================================================");
		System.in.read();
		
		System.out.println("C-LRU on Index Join");
		start = System.currentTimeMillis();
		size = 3; // tighter memory constraints for index search. 
		m = new MemoryManager(policy, size);
		c = ((ChainedLeastRecentlyUsedReplacementPolicy)policy).love(true);
		new AccessPatterns(m).trace_index_join(c.id());
		end = System.currentTimeMillis();
		
		m.stats();
		System.out.println("duration = " + (end - start));
		System.out.println("===================================================");
		System.in.read();
		
		System.out.println("LRU on Index Join");
		start = System.currentTimeMillis();
		m = new MemoryManager(new LRUReplacementPolicy(), size);
		new AccessPatterns(m).trace_index_join(5);
		end = System.currentTimeMillis();
		
		m.stats();
		System.out.println("duration = " + (end - start));
		System.out.println("===================================================");
		System.in.read();
	}
}
