package util;

public class LogicalClock implements IClock {

	private long counter = 0;
	
	@Override
	public long current() {
		return this.counter;
	}

	@Override
	public long generate() {
		return ++this.counter;
	}

}
