package mem;

import java.util.BitSet;
import java.util.Iterator;

public class Memory implements Iterable<Page> {

	private Page [] pages = null;
	private BitSet bitmap = null;
	
	/**
	 * Creates a new Physical Memory Object.
	 * @param size memory capacity in blocks.
	 */
	public Memory(int size) {
		this.pages = new Page[size];
		this.bitmap = new BitSet(size);
	}
	
	/**
	 * Stores a page to physical memory.
	 * @param page page to be stored.
	 * @param frame frame number to store page.
	 * @return swapped page.
	 */
	public Page store(Page page, int frame) {
		
		Page swapped = this.pages[frame];
		
		if (page != null) {
			page.frame = frame;
		}
		
		this.pages[frame] = page;
		this.bitmap.set(frame, page != null);
		
		return page == swapped ? null : swapped;
	}
	
	/**
	 * Loads a page from physical memory.
	 * @param frame physical frame number.
	 * @return page stored at given frame.
	 */
	public Page load(int frame) {
		if (this.bitmap.get(frame)) {
			return this.pages[frame];
		} else {
			return null;
		}
	}
	
	/**
	 * Gets the next available frame. -1 if no frames are available, i.e,
	 * physical memory is full.
	 * @return number of free frame.
	 */
	public int free_frame() {
		if (this.bitmap.cardinality() < this.pages.length) {
			return this.bitmap.nextClearBit(0);
		} else {
			return -1;
		}
	}

	@Override
	public Iterator<Page> iterator() {
		return new MemoryIterator(this.pages, this.bitmap);
	}
	
	class MemoryIterator implements Iterator<Page> {

		private Page [] pages = null;
		private int counter = 0;
		private BitSet bitmap;
		
		private MemoryIterator(Page [] pages, BitSet bitmap) {
			this.pages = pages;
			this.bitmap = bitmap;
		}
		
		@Override
		public boolean hasNext() {
			return this.bitmap.nextSetBit(this.counter) != -1;
			
		}

		@Override
		public Page next() {
			int index = this.bitmap.nextSetBit(this.counter);
			if (index != -1) {
				this.counter = index;
			}
			
			return this.pages[this.counter++];
		}

		@Override
		public void remove() {
			throw new UnsupportedOperationException();
		}
		
	}
}
