package mem;

import java.util.HashMap;

public class PageTable {

	private HashMap<Integer, Integer> mapping = new HashMap<Integer, Integer>();
	
	void add(int address, int frame) {
		this.mapping.put(address, frame);
	}
	
	void remove(int address) {
		this.mapping.remove(address);
	}
	
	int lookup(int address) {
		Integer frame = this.mapping.get(address);			
		return frame != null ? frame : -1;
	}
}
