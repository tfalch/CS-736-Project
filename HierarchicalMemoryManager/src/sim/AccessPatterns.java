package sim;

import java.util.Random;

import mem.IMemoryManager;
import mem.MemoryChain;
import mem.MemoryChain.EvictionPolicy;

public class AccessPatterns {

	private IMemoryManager mem_mgr = null;
	
	public AccessPatterns(IMemoryManager m) {
		this.mem_mgr = m;
	}
	
	/**
	 * simulates nested block join. 
	 * @param r - # of blocks in R-relation (r << s).
	 * @param s - # of blocks in S-relation (s >> r).
	 */
	public void trace_block_join(int r, int s) {
		
		MemoryChain chain = this.mem_mgr.createMemoryChain(EvictionPolicy.MRU);
		
		int [] R = {0, r}; // R relation block #s.
		int [] S = {r + s, r + s + s}; // S relation block #s.
		 
		for (int i = R[0]; i < R[1]; ) {
			int n = 0;
			int l = i;
			
			/* simulate retrieving R blocks from disk into memory */
			for (int j = 0; j < this.mem_mgr.capacity() - 1 && i < R[1]; j++, i++) {
				this.mem_mgr.access(i);
				this.mem_mgr.link(chain, i);
				n++;
			}
			
			for (int j = S[0]; j < S[1]; j++) {
				/* load j from disk. */
				this.mem_mgr.access(j);
				/* compare records in R & S */
				for (int b = l; b < i; b++) {
					this.mem_mgr.access(b);
					this.mem_mgr.access(j);
				}

				this.mem_mgr.unlink(j);
			}
			
			/* hate pages to flush from cash */
			this.mem_mgr.breakMemoryChain(chain);
		}
		
		this.mem_mgr.releaseMemoryChain(chain);
	}
	
	/**
	 * simulates index join.
	 * @param k - # of keys to simulate searching.
	 * @param n - # of records in index/relation.
	 * @param f - fan-out/branching factor of index.
	 */
	public void trace_index_join(int k, int n, int f) {
		
		Random r = new Random();
		int d = (int)Math.ceil(Math.log(n) / Math.log(f));
		
		int rcardinality = (int)Math.ceil(n/Math.pow(f, d-1));
		int capacity = (d+1)*(1+rcardinality+1);
		MemoryChain chain = this.mem_mgr.createMemoryChain(EvictionPolicy.MRU, capacity);
		
		this.mem_mgr.access(0); // load root.
		this.mem_mgr.anchor(chain, 0);
		
		// generate x random search keys.
		for (int i = 0; i < k; i++) {
			int branch = r.nextInt((int)(Math.ceil(n/Math.pow(f, d-1))));
			int block = 0;				
			
			this.mem_mgr.access(block); // read root.
			for (int j = 1; j < d; j++) {
				branch = ((1+branch) * f) + r.nextInt(f);
				block = branch / f;
				
				this.mem_mgr.access(block); // read next node.
				if (j == 1)
					this.mem_mgr.link(chain, block);
				else 
					this.mem_mgr.unlink(block);
			}
			
			// last branch is record id.
			// block = branch / #recs-per-block.
			this.mem_mgr.access(branch/40); // read data block.
		}
		
		this.mem_mgr.releaseMemoryChain(chain);
	}
	
	/**
	 * simulates naive matrix multiplication. used for benchmarking
	 * various approaches.
	 * @param m m dimension of matrix.
	 * @param n n dimension of matrix.
	 */
	public void trace_naive_matrix_multiplication(int m, int n) {
		MemoryChain chain = this.mem_mgr.createMemoryChain(EvictionPolicy.MRU);
		
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < m; j++) {
				for (int k = 0; k < n; k++) {
					
					int A = i * n + k;
					int B = j * n + k + (m * n);
					
					this.mem_mgr.access(A);
					if (j == 0)
						this.mem_mgr.link(chain, A);
					this.mem_mgr.access(B);
				}
			}
			
			this.mem_mgr.breakMemoryChain(chain);
		}
	}
}
