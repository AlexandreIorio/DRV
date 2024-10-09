#include <stdio.h>
#include <stdint.h>

int main()
{
	uint32_t value = 1;
	int shift;

	printf("Enter the number of bits to shift: ");
	int read = scanf("%d", &shift);

	uint32_t result = value << shift;

	printf("Result of %u left shifts bits of %d is : %d\n", shift, value,
	       result);

	return 0;
}
