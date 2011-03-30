package app;

import java.util.Random;

import mmu.MemoryManager;

public class AccessPatterns {

	private MemoryManager m = null;
	
	public AccessPatterns(MemoryManager m) {
		this.m = m;
	}
	
	/**
	 * simulates nested block join. 
	 * @param l
	 */
	public void trace_block_join(int l) {
		
		int [] R = {0, 50}; // R relation block ids.
		int [] S = {100, 500}; // S relation block ids.
 		
		
		for (int i = R[0]; i < R[1]; ) {
			int n = 0;
			
			/* simulate retrieving R blocks from disk into memory */
			for (int j = 0; j < this.m.capacity() - 1 && i < R[1]; j++, i++) {
				this.m.access(i);
				this.m.love(i, l);
				n++;
			}
			
			for (int j = S[0]; j < S[1]; j++) {
				
				for (int k = 1; k <= n; k++) {
					this.m.access(i-k);
					this.m.access(j);
				}
			}
		}
	}
	
	public void trace_index_join(int l) {
		int d = 4; // tree depth
		int f = 255; // branching factor
		
		Random r = new Random();
		
		this.m.access(0); // load root.
		this.m.love(0, l, true); // pin root of tree in memory.
		
		// generate x random search keys.
		for (int i = 0; i < 250; i++) {
			int branch = 0;
		
			this.m.access(branch); // read root.
			for (int j = 1; j < d; j++) {
				branch = 1 + branch * f + r.nextInt(f);
				
				this.m.access(branch);
				this.m.love(branch, l);
			}
		}
	}
	
}
