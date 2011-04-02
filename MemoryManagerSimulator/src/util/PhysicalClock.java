package util;

public class PhysicalClock implements IClock {

	private long timestamp = 0;
	@Override
	public long current() {
		return this.timestamp;
	}

	@Override
	public long generate() {
		return this.timestamp = System.currentTimeMillis();
	}

}
