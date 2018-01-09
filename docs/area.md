# Docs

## Area Define

	As areas are quite few compared to records, we use FIXED-LENGTH strings in area definition.

	Bit 0..15: Length (FIXED).
	Bit 16..31: Type ID. (See area.h for details.)
	Bit 32..47: Offset. (Column position - Record beginning position)
	Bit 48..63: Padding. (Content for output.)
	Bit 64..2131: Column name.
	Bit 2132..4159: Table name.
	Bit 4160..4167: Notnull restriction.
	Bit 4168..4176: Primary restriction.

## Area Record

### Int

	Bit 0..15: Length of this record. (In bytes.)
	Bit 16..23: Null flag, 1 means null. (Actually 1 bit is better, but it is too hard to manage when data are not aligned to bytes.)
	Bit 24..55: Int. (Notice: NOT ALIGNED.)

### String
	
	Bit 0..15: Length of this record. (In bytes.)
	Bit 16..23: Null flag, 1 means null. (Actually 1 bit is better, but it is too hard to manage when data are not aligned to bytes.)
	Bit 24..?: Char data.
