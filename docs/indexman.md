# Docs

## Index Manager

### Pages Management

	Page 0: Info Page.
		Bit 0..31: Root Page.
		Bit 32..63: Depth. (0 means son of root is a leaf)
		Bit 65504..65535: Number of pages.

	Leaf Page:
		Bit 65520..65535: Begin of empty area (in bytes).
		Bit 65504..65519: Number of slots.
		The previous (number of slots) * 16 bits represents the start of each entry.
		Each entry has its first 16 bits represents its length, and the following 48 bits represents its key. As a result, each entry has a length of at least 64 bit.

	Inner Page:
		Bit 0..15: Number of subpages.
		Bit 16..47: First subpage.
		After that, each 80 bits represents a (48, 32) - (Key, subpage_ID) pair.
		Key is defined the same as RecordPos. Notice RecordPos has 64 bits, but we only store the first 48 bits.
