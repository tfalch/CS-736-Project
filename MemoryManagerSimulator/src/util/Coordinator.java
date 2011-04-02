package util;

public class Coordinator {

	private static IClock clock = new LogicalClock();
	
	public static long nextSequence() {
		return Coordinator.clock.generate();
	}
	
	public static long currentSequence() {
		return Coordinator.clock.current();
	}
}
