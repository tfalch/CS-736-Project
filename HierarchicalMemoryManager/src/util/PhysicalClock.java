package util;

public class PhysicalClock implements IClock {

	private long timestamp = 0;
	
	/* (non-Javadoc)
	 * @see util.IClock#generate()
	 */
	@Override
	public long current() {
		return this.timestamp;
	}

	/**
	 * Gets the next sequence value. The sequence value is guaranteed to be
	 * non-decreasing only, (i.e not unique) between subsequent generate calls.
	 * @return sequence value. 
	 */
	@Override
	public long generate() {
		return this.timestamp = System.currentTimeMillis();
	}

}
