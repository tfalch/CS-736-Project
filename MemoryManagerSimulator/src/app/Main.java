package app;
import java.io.IOException;
import java.text.DecimalFormat;

import mmu.*;
import mmu.policy.ChainedLeastRecentlyUsedReplacementPolicy;
import mmu.policy.ClockReplacementPolicy;
import mmu.policy.IPageReplacementPolicy;
import mmu.policy.LRUReplacementPolicy;
import mmu.policy.LoveClockReplacementPolicy;
import mmu.policy.LoveHateReplacementPolicy;
import mmu.policy.MultiLevelQueueReplacementPolicy;

public class Main {	
	
	private static void simulate_block_join(IPageReplacementPolicy p, int l, boolean verbose) {

		int noOfRuns = 1;
		double sumOfMissRatios = 0;
		double minMissRatio = 1.0;
		double maxMissRatio = 0.0;
		
		for(int i = 0; i < noOfRuns; i++){
			long start = System.currentTimeMillis();
			MemoryManager m = new MemoryManager(p, 30);
			new AccessPatterns(m).trace_block_join(l, 35, 50, 0); //365, 13530, 0);
			long end = System.currentTimeMillis();
			
			if(verbose || true){
				System.out.print(p.name() + ": ");
				System.out.print("duration=" + (end - start) + ";"); m.summary();
			}
			double missRatio = m.getMissRatio();
			
			sumOfMissRatios += missRatio;
			
			if(missRatio < minMissRatio)
				minMissRatio = missRatio;
			
			if(missRatio > maxMissRatio)
				maxMissRatio = missRatio;
			
		}
		DecimalFormat df = new DecimalFormat("0.00");
		
		System.out.print(p.name() + ":\t");
		System.out.print("min: " + df.format(minMissRatio));
		System.out.print("  max: " + df.format(maxMissRatio));
		System.out.print("  avg: " + df.format(sumOfMissRatios/noOfRuns));
		System.out.println();
		
	}
	
	private static void simulate_index_join(IPageReplacementPolicy p, int l, boolean verbose) {
		
		//TODO Doesn't work with noOfRuns more than 1, must reset the policies or something...
		int noOfRuns = 1;
		double sumOfMissRatios = 0;
		double minMissRatio = 1.0;
		double maxMissRatio = 0.0;
		
		for(int i = 0; i < noOfRuns; i++){
			long start = System.currentTimeMillis();
			MemoryManager m = new MemoryManager(p, 15);
			new AccessPatterns(m).trace_index_join(l, 2500, 100000000, 64);
			long end = System.currentTimeMillis();
			
			if(verbose){
				System.out.print(p.name() + ": ");
				System.out.print("duration=" + (end - start) + ";"); m.summary();
			}
			double missRatio = m.getMissRatio();
			
			sumOfMissRatios += missRatio;
			
			if(missRatio < minMissRatio)
				minMissRatio = missRatio;
			
			if(missRatio > maxMissRatio)
				maxMissRatio = missRatio;
			
		}
		DecimalFormat df = new DecimalFormat("0.00");
		
		System.out.print(p.name() + ":\t");
		System.out.print("min: " + df.format(minMissRatio));
		System.out.print("  max: " + df.format(maxMissRatio));
		System.out.print("  avg: " + df.format(sumOfMissRatios/noOfRuns));
		System.out.println();
	}
	
	private static void simulate_block_join() {

		long start = System.currentTimeMillis();
		MultiLevelMemoryManager m = new MultiLevelMemoryManager(30);
		new AccessPatterns(m).trace_block_join(m.chain().id(), 365, 13530);
		long end = System.currentTimeMillis();
		
		System.out.print("MLMU: ");
		System.out.print("duration=" + (end - start) + ";"); m.summary();
	}

	
	public static void run() {
		
		System.out.println("\t\t===================================================");
		System.out.println("\t\t\t   Simulating Nested Block Join");
		System.out.println("\t\t===================================================");		
		
		
		ChainedLeastRecentlyUsedReplacementPolicy clru = new ChainedLeastRecentlyUsedReplacementPolicy();
		simulate_block_join(clru, clru.love().id(), false);
		simulate_block_join(new LoveHateReplacementPolicy(), 254, false);
		simulate_block_join(new LRUReplacementPolicy(), -1, false);
		simulate_block_join(new ClockReplacementPolicy(), -1, false);
		simulate_block_join(new LoveClockReplacementPolicy(), 254, false);
		simulate_block_join(new MultiLevelQueueReplacementPolicy(), 254, false);
		Main.simulate_block_join();
		
		System.out.println("\t\t===================================================");
		System.out.println("\t\t\t       Simulating Index Join");
		System.out.println("\t\t===================================================");
		
		clru = new ChainedLeastRecentlyUsedReplacementPolicy();
		simulate_index_join(clru, clru.love(true).id(), false);
		simulate_index_join(new LoveHateReplacementPolicy(), 254, false);
		simulate_index_join(new LRUReplacementPolicy(), -1, false);
		simulate_index_join(new ClockReplacementPolicy(), -1, false);
		simulate_index_join(new LoveClockReplacementPolicy(), 254, false);
		simulate_index_join(new MultiLevelQueueReplacementPolicy(), 254, false);
		
		
		System.out.println();
	}
	
	public static void main(String [] args) throws IOException {			
		Main.run();
	}
}
