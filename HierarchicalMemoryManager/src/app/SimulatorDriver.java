package app;

import mem.HierarchicalMemoryManager;
import sim.AccessPatterns;
import sim.Experiment;
import sim.Experiment.Trace;

public class SimulatorDriver {

	private Experiment [] experiments;
	
	public SimulatorDriver(Experiment [] experiments) {
		this.experiments = experiments;
	}
	
	private void run(Experiment experiment) {

		System.out.println("Running Experiment " + experiment.traceToRun().name() + " " +
				experiment.noOfRuns() + (experiment.noOfRuns() == 1 ? " time" : " times"));
		
		for (int i = 0; i < experiment.noOfRuns();) {
			System.out.print("\trun " + ++i + " of " +  experiment.noOfRuns() + ": ");
			switch (experiment.traceToRun()) {
			case BLOCK_JOIN:
				this.simulate_block_join(experiment);
				break;
			case INDEX_JOIN:
				this.simulate_index_join(experiment);
				break;
			case MATRIX_MULTIPLICATON:
				this.simulate_naive_matrix_multiply(experiment);
				break;
			}
		}
		
		System.out.println("End-of-Experiment");
		System.out.println();
	}
	
	public void start() {
		
		for (int i = 0; i < this.experiments.length; i++) {
			this.run(this.experiments[i]);
		}
	}
	
	private void simulate_block_join(Experiment experiment) {

		int [] arguments = experiment.arguments();
		long start = System.currentTimeMillis();
		HierarchicalMemoryManager m = new HierarchicalMemoryManager(
				arguments[Experiment.NestedBlockJoinArgument.MEMORY_SIZE.ordinal()]);
		new AccessPatterns(m).trace_block_join(
				arguments[Experiment.NestedBlockJoinArgument.BLOCK_SIZE_R.ordinal()], 
				arguments[Experiment.NestedBlockJoinArgument.BLOCK_SIZE_S.ordinal()]);
		long end = System.currentTimeMillis();
		
		System.out.print("duration=" + (end - start) + ";"); m.summary();
	}
	
	private void simulate_index_join(Experiment experiment) {

		int [] arguments = experiment.arguments();
		
		long start = System.currentTimeMillis();
		HierarchicalMemoryManager m = new HierarchicalMemoryManager(
				arguments[Experiment.IndexJoinArgument.MEMORY_SIZE.ordinal()]);
		new AccessPatterns(m).trace_index_join(
				arguments[Experiment.IndexJoinArgument.NUM_LOOKUPS.ordinal()], 
				arguments[Experiment.IndexJoinArgument.NUM_RECORDS.ordinal()],
				arguments[Experiment.IndexJoinArgument.KEYS_PER_NODE.ordinal()]);
		long end = System.currentTimeMillis();
		
		System.out.print("duration=" + (end - start) + ";"); m.summary();
	}
	
	private void simulate_naive_matrix_multiply(Experiment experiment) {
		
		int [] arguments = experiment.arguments();
		
		long start = System.currentTimeMillis();
		HierarchicalMemoryManager m = new HierarchicalMemoryManager(
				arguments[Experiment.MatrixMultiplicationArgument.MEMORY_SIZE.ordinal()]);
		new AccessPatterns(m).trace_naive_matrix_multiplication(
				arguments[Experiment.MatrixMultiplicationArgument.M.ordinal()], 
				arguments[Experiment.MatrixMultiplicationArgument.N.ordinal()]);
		long end = System.currentTimeMillis();
		
		System.out.print("duration=" + (end - start) + ";"); m.summary();
	}
	
	public static void main(String [] args) {
		
		int n = 0;
		int [] arguments = null;
		
		Experiment [] experiments = new Experiment[4];
		
		arguments = new int[3];
		arguments[Experiment.NestedBlockJoinArgument.MEMORY_SIZE.ordinal()] = 5; //30;
		arguments[Experiment.NestedBlockJoinArgument.BLOCK_SIZE_R.ordinal()] = 12; //300;
		arguments[Experiment.NestedBlockJoinArgument.BLOCK_SIZE_S.ordinal()] = 12; //15000;
		
		experiments[n++] = new Experiment(Trace.BLOCK_JOIN, 10, arguments);
		
		arguments = new int[4];
		arguments[Experiment.IndexJoinArgument.MEMORY_SIZE.ordinal()] = 10;
		arguments[Experiment.IndexJoinArgument.NUM_RECORDS.ordinal()] = 100000000;
		arguments[Experiment.IndexJoinArgument.NUM_LOOKUPS.ordinal()] = 2500;
		arguments[Experiment.IndexJoinArgument.KEYS_PER_NODE.ordinal()] = 255;
		
		experiments[n++] = new Experiment(Trace.INDEX_JOIN, 10, arguments);
		
		arguments[Experiment.IndexJoinArgument.KEYS_PER_NODE.ordinal()] = 32;
		experiments[n++] = new Experiment(Trace.INDEX_JOIN, 10, arguments);
		
		arguments = new int[3];
		arguments[Experiment.MatrixMultiplicationArgument.MEMORY_SIZE.ordinal()] = 5;
		arguments[Experiment.MatrixMultiplicationArgument.M.ordinal()] = 3;
		arguments[Experiment.MatrixMultiplicationArgument.N.ordinal()] = 4;
		
		experiments[n++] = new Experiment(Trace.MATRIX_MULTIPLICATON, 1, arguments);
		
		new SimulatorDriver(experiments).start();
	}
}