#include <stdio.h>
#include <stdlib.h>
// inculde to have uint32_t
#include <stdint.h>

union MyUnion {
    int value;
};

int main() {
    union MyUnion u;
    u.value = 0xBEEFCAFE;

    char *ptr = (char *)&u.value;

    printf("Check endianness\n");
    for (int i = 0; i < sizeof(u.value); i++)
    {
        printf("%x\n", *ptr);
        ptr ++;
    }

    ptr = (char *)&u.value;
    ptr += sizeof(u.value) - 1;

    printf("invert value\n");
    for (int i = 0; i < sizeof(u.value); i++)
    {
        printf("%x\n", *ptr);
        ptr --;
    }
    return 0;
}