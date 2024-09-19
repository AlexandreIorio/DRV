#ifdef second_h
#define second_h

#include <stdio.h>

#include "first.h"

#define SECOND_NAME "second"

int id2 = 2;

void second()
{
	printf("I am %s (id = %d) and I introduce no-one :(\n", SECOND_NAME, id2);
}

#endif // SECOND_H