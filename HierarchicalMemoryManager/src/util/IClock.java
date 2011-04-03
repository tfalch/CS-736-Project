package util;

public interface IClock {

	/**
	 * Gets the current value of the clock.
	 * @return current value of the clock.
	 */
	public long current();
	
	/**
	 * Gets the next sequence value. Sequence values are guaranteed to be non-decreasing
	 * and depending on the Clock implementation, sequence values may also be 
	 * but are not guaranteed to be unique between subsequent calls.  
	 * @return next sequence value.
	 */
	public long generate();
}
