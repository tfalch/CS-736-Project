package sim;

public class Experiment {
	
	public enum Trace {
		BLOCK_JOIN ("Nested Block Join"),
		INDEX_JOIN ("Index Join");
		
		String name;
		Trace(String name) {
			this.name = name;
		}
		
		public String getName() {
			return name;
		}
	};
	
	public enum IndexJoinArgument {
		MEMORY_SIZE,
		NUM_RECORDS,
		KEYS_PER_NODE,
		NUM_LOOKUPS
	};
	
	public enum NestedBlockJoinArgument {
		MEMORY_SIZE,
		BLOCK_SIZE_R,
		BLOCK_SIZE_S
	};
	
	Trace traceToRun;
	int noOfRuns;
	int [] arguments;
	
	/**
	 * Creates a new instance.
	 * @param traceToRun Experiment trace.
	 * @param noOfRuns Number of times experiment should be executed.
	 * @param arguments Arguments used in experiment. Please use the 
	 * appropriate arg-enum to initialize the argument values.
	 */
	public Experiment(Trace traceToRun, int noOfRuns, int [] arguments) {
		this.traceToRun = traceToRun;
		this.noOfRuns = noOfRuns;
		this.arguments = arguments;
	}
	
	public Trace traceToRun() {
		return this.traceToRun;
	}
	
	public int noOfRuns() {
		return this.noOfRuns;
	}
	
	public int [] arguments() {
		return this.arguments;
	}
};
