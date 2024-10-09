#include <stdio.h>

/* this is a named structure we can use multllple times */
struct a {
    int b;
    char c;
};

int main() {
    struct a instance1;
    instance1.b = 42;
    instance1.c = 'A';

    printf("first case :\n");
    printf("b = %d, c = %c\n", instance1.b, instance1.c);

    /* this is an anonymous structure we can just use this instance */
    struct {
        int b;
        char c;
    } a;

    a.b = 84;
    a.c = 'Z';
    printf("second case :\n");
    printf("b = %d, c = %c\n", a.b, a.c);

    return 0;
}
