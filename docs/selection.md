# Docs

## Selection

### Translate

	Translate input expr into lambda functions.

	These functions will be parsed to record manager in order to cut off memory usage on select operation.

### Check
	
	Check whether selected records (especially joined records) satisfies where expression.

	Different from translate, check performs after candidates loaded into memory.

	CheckIterate performs multi-level for when joining multiple (>2) tables.
