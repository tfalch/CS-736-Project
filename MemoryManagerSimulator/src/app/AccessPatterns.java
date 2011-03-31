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
	 * @param l - love.
	 * @param r - # of blocks in R-relation (r << s).
	 * @param s - # of blocks in S-relation (s >> r).
	 */
	public void trace_block_join(int l, int r, int s) {
		
		int [] R = {0, r}; // R relation block ids.
		int [] S = {r + s, r + s + s}; // S relation block ids.
		
		for (int i = R[0]; i < R[1]; ) {
			int n = 0;
			
			/* simulate retrieving R blocks from disk into memory */
			for (int j = 0; j < this.m.capacity() - 1 && i < R[1]; j++, i++) {
				this.m.access(i);
				this.m.love(i, l);
				n++;
			}

			for (int j = S[0]; j < S[1]; j++) {
				/* compare records in R & S */
				for (int k = 1; k <= n; k++) {
					this.m.access(j);
					this.m.access(i-k);
				}
			}
			
			/* hate pages to flush from cash */
			for (int k = 1; k <= n; k++) {
				this.m.hate(i-k, 0);
			}
		}
	}
	
	/**
	 * 
	 * @param l - love
	 * @param k - # of keys to simulate searching.
	 * @param n - # of records in index/relation.
	 * @param f - fan-out/branching factor of index.
	 */
	public void trace_index_join(int l, int k, int n, int f) {
		
		Random r = new Random();
		int d = (int)Math.ceil(Math.log(n) / Math.log(f));
		
		this.m.access(0); // load root.
		this.m.love(0, l, true); // pin root of tree in memory.
		
		// generate x random search keys.
		for (int i = 0; i < k; i++) {
			int branch = r.nextInt((int)(Math.ceil(n/Math.pow(f, d-1))));
			int block = 0;				
			
			this.m.access(block); // read root.
			for (int j = 1; j < d; j++) {
				branch = ((1+branch) * f) + r.nextInt(f);
				block = branch / f;
				
				this.m.access(block); // read next node.
				this.m.love(block, l);
			}
			
			// last branch is record id.
			// block = branch / #recs-per-block.
			this.m.access(branch/40); // read data block. 
		}
	}
}
