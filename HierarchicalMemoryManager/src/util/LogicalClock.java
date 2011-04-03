package util;

public class LogicalClock implements IClock {

	private long counter = 0;
	
	/* (non-Javadoc)
	 * @see util.IClock#generate()
	 */
	@Override
	public long current() {
		return this.counter;
	}
	
	/**
	 * Gets the next sequence value which is guaranteed to be unique and
	 * non-decreasing.
	 * @return unique and non-decreasing sequence value.
	 */
	@Override
	public long generate() {
		return ++this.counter;
	}

}
