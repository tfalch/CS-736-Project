package app;
import mmu.LoveHateReplacementPolicy;
import mmu.MemoryManager1;


public class Main {

	public static void main(String [] args) {
		
		int [] R = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
		int [] S = {101, 102, 103, 104, 105, 106, 107, 108, 109, 110,
				    111, 112, 113, 114, 115, 116, 117, 118, 119, 120,
				    121, 122, 123, 124, 125, 126, 127, 128, 129, 130,
				    131, 132, 133, 134, 135, 136, 137, 138, 139, 140};
		
		int size = 5;
		
		LoveHateReplacementPolicy m = new LoveHateReplacementPolicy(size);
		
		for (int i = 0; i < R.length; ) {
			int n = 0;
			/* load first four blocks into memory */
			for (int j = 0; j < m.capacity() - 1 && i < R.length; j++, i++) {
				m.access(R[i]);
				m.love(R[i], 20); /* m.love(R[i], 5); */
				n++;
			}
			
			for (int j = 0; j < S.length; j++) {
				m.access(S[j]);
				for (int k = 1; k <= n; k++) {
					m.access(R[i-k]);
					m.access(S[j]);
				}
			}
		}
		
		m.stats();
		System.out.println();
		
		MemoryManager1 mmu1 = new MemoryManager1(size);
		
		int id = mmu1.love();
		
		for (int i = 0; i < R.length; ) {
			int n = 0;
			/* load first four blocks into memory */
			for (int j = 0; j < m.capacity() - 1 && i < R.length; j++, i++) {
				mmu1.access(R[i]);
				mmu1.love(R[i], id); 
				n++;
			}
			
			for (int j = 0; j < S.length; j++) {
				mmu1.access(S[j]);
				for (int k = 1; k <= n; k++) {
					mmu1.access(R[i-k]);
					mmu1.access(S[j]);
				}
			}
		}
		
		mmu1.stats();
		System.out.println();
	}
}
