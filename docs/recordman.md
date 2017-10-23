# Docs

## Record Manager

### Pages Management

	Flag page: first at Page 0, every FILE_STEP pages.
		Bit 0..(FILE_STEP-1): Flags if next pages are full.
			Bit i represents the (i+1)th page after this page.
			0 means empty, 1 means full.
			Default: 0.
			Whenever an attempt to allocate a slot in one page failed, change its bit to 1.
			Whenever a slot is removed from one page, change its bit to 0.
			An attempt to allocate will only search in slots whose bits are 0.
		Bit 65520..65535: The number of pages allocated after this page.
			0 When the flag page is created.
			Whenever an attempt to allocate a slot in existing pages failed, a new page needs to be created and this number is added by 1.
			When this number is FILE_STEP-1 and needs to grow, a new flag page needs to be created.
			As a result, when its value equals to FILE_STEP, it means that another flag page is created FILE_STEP pages after this page.
			So it also acts as a flag of a linked-list.
			Notice: as this number occupys 16 bit, FILE_STEP is logically at most 65520.

	Slot page:
		Page that store slots of real data.
		The indexes starts from back while slots of real data starts from front.
		Each index occupies an unsigned short, and represents the END of a slot (in bytes, not bit).
		An index with value 65535(-1) means removed.
		The last unsigned short (i.e. bit 65520..65535) represents the length of index.
		We need extra 16 bit to store a slot.
		When we definitely REMOVE a slot or CHANGE ITS LENGTH, all the slots after it needs shifting, as well as indexes.
		(Not so slow compared to writeback...)
