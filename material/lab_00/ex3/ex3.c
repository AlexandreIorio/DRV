#include <stdio.h>

int main()
{

    int value = 0xbeefcafe;
    char *ptr = (char *)&value;

    printf("Check endianness\n");
    for (int i = 0; i < sizeof(value); i++)
    {
        printf("%x\n", *ptr);
        ptr ++;
    }

    ptr = (char *)&value;
    ptr += sizeof(value) - 1;

    printf("invert value\n");
    for (int i = 0; i < sizeof(value); i++)
    {
        printf("%x\n", *ptr);
        ptr --;
    }
}

