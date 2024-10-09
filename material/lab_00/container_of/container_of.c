#include <stdio.h>
#include <stddef.h>

#define container_of(ptr, type, member) \
	(type *)(void *)((char *)ptr - offsetof(type, member))

struct Struct {
	int a;
	char b;
	double c;
};

struct StructPacked {
	int a;
	char b;
	double c;
} __attribute((__packed__));

int main()
{
	struct Struct s;

	// get addresses of the members
	int *pa = &s.a;
	char *pb = &s.b;
	double *pc = &s.c;

	// Use container_of to get the address of the structure
	struct Struct *from_a = container_of(pa, struct Struct, a);
	struct Struct *from_b = container_of(pb, struct Struct, b);
	struct Struct *from_c = container_of(pc, struct Struct, c);

	struct StructPacked s_packed;

	int *pa_packed = &s_packed.a;
	char *pb_packed = &s_packed.b;
	double *pc_packed = &s_packed.c;

	struct StructPacked *from_a_packed =
		container_of(pa_packed, struct StructPacked, a);
	struct StructPacked *from_b_packed =
		container_of(pb_packed, struct StructPacked, b);
	struct StructPacked *from_c_packed =
		container_of(pc_packed, struct StructPacked, c);

	printf("Origina address of struct: \t %p\n", &s);
	printf("Address from a:\t\t\t %p\n", from_a);
	printf("Address from b:\t\t\t %p\n", from_b);
	printf("Address from c:\t\t\t %p\n", from_c);
	printf("Offset of a: %ld\n", offsetof(struct Struct, a));
	printf("Offset of b: %ld\n", offsetof(struct Struct, b));
	printf("Offset of c: %ld\n", offsetof(struct Struct, c));

	printf("Origina address of struct packed:%p\n", &s_packed);
	printf("Address from a packed:\t\t %p\n", from_a_packed);
	printf("Address from b packed:\t\t %p\n", from_b_packed);
	printf("Address from c packed:\t\t %p\n", from_c_packed);
	printf("Offset of a packed: %ld\n", offsetof(struct StructPacked, a));
	printf("Offset of b packed: %ld\n", offsetof(struct StructPacked, b));
	printf("Offset of c packed: %ld\n", offsetof(struct StructPacked, c));

	return 0;
}
