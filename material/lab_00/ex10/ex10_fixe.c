#include <stdint.h>
#include <stdio.h>

int main()
{
	uint32_t value = 1;
	int shift = 42;
	uint32_t result = value << shift;

	printf("Result of %u left shifts bits of %d is : %d\n", shift, value,
	       result);
	return 0;
}