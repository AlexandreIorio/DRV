#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* Taken from:
 * https://hps.vi4io.org/_media/teaching/wintersemester_2012_2013/epc-1213-grigoras-oo-praesentation.pdf
 * (and slightly modified)
 */

/* Shape abstract interface */
struct ShapeFuncTable;
struct Shape {
	struct ShapeFuncTable *funcTable;
};

struct ShapeFuncTable {
	void (*printArea)   (struct Shape *obj);
	void (*moveTo)      (struct Shape *obj, int newx, int newy);
	void (*destructor_) (struct Shape *obj);
};

struct Shape *Shape_init () { assert (0); }
void Shape_destroy(__attribute__ ((unused)) struct Shape *obj) { }

/* Volume abstract interface */

struct VolumeFuncTable;
struct Volume {
	struct Shape *super;
	struct VolumeFuncTable *funcTable;
};

struct VolumeFuncTable {
	void (*printVolume) (struct Volume *obj);
	void (*destructor_) (struct Volume *obj);
};

struct Volume *Volume_init () { assert (0); }
void Volume_destroy(__attribute__ ((unused)) struct Volume *obj) { }

/* Rectangle class */
struct Rectangle {
	struct Shape super;
	int x;
	int y;
	int width;
	int height;
};

void Rectangle_printArea(struct Shape *obj)
{
	struct Rectangle *rdata = (struct Rectangle *) obj;
	int area = rdata->width * rdata->height;
	printf("Rectangle area is %d\n", area);
}

/* this method must not print it just need to move rectangle, because i reuse it in parallelepiped*/
void Rectangle_moveTo(struct Shape *obj,
		      int newx,
		      int newy)
{
	struct Rectangle *rdata = (struct Rectangle *) obj;
	rdata->x = newx;
	rdata->y = newy;
	printf("Moving your rectangle to (%d, %d)\n",
	       rdata->x, rdata->y);
}

void Rectangle_setWidth(struct Shape *obj, int neww)
{
	struct Rectangle *rdata = (struct Rectangle *) obj;
	rdata->width = neww;
}


void Rectangle_destroy(struct Shape *obj)
{
	Shape_destroy(obj);
	free(obj);
}

struct RectangleFuncTable {
	struct ShapeFuncTable super;
	void (*setWidth) (struct Shape *obj, int neww);
} rectangleFuncTable = {
			{
			 .printArea = Rectangle_printArea,
			 .moveTo = Rectangle_moveTo,
			 .destructor_ = Rectangle_destroy
			},
			.setWidth = Rectangle_setWidth
};

struct Shape* Rectangle_init(int initx, int inity,
			     int initw, int inith)
{
	struct Rectangle *obj =
		(struct Rectangle *) malloc(sizeof(struct Rectangle));
	obj->super.funcTable =
		(struct ShapeFuncTable *) &rectangleFuncTable;
	obj->x = initx;
	obj->y = inity;
	obj->width = initw;
	obj->height = inith;
	return (struct Shape *) obj;
}

/* Circle class */
struct Circle {
	struct Shape super;
	int x;
	int y;
	int radius;
};

void Circle_printArea(struct Shape *obj)
{
	struct Circle *cdata = (struct Circle *) obj;
	double area = 3.14 * cdata->radius * cdata->radius;
	printf("Circle area is %.2f\n", area);
}

void Circle_moveTo(struct Shape *obj, int newx, int newy)
{
	struct Circle *cdata = (struct Circle *) obj;
	cdata->x = newx;
	cdata->y = newy;
	printf("Moving your circle to (%d, %d)\n", cdata->x, cdata->y);
}

void Circle_destroy(struct Shape *obj)
{
	Shape_destroy(obj);
	free(obj);
}

struct CircleFuncTable {
	struct ShapeFuncTable super;
} circleFuncTable = {
		     {
		      .printArea = Circle_printArea,
		      .moveTo = Circle_moveTo,
		      .destructor_ = Circle_destroy
		     }
};

struct Shape *Circle_init(int initx, int inity, int initr)
{
	struct Circle *obj =
		(struct Circle *) malloc(sizeof(struct Circle));
	obj->super.funcTable =
		(struct ShapeFuncTable *) &circleFuncTable;
	obj->x = initx;
	obj->y = inity;
	obj->radius = initr;
	return (struct Shape *) obj;
}

/* Parallelepiped class */
struct Parallelepiped {
	struct Volume super;
	int z;
};

void Parallelepiped_print_volume(struct Volume *obj) {
	struct Parallelepiped *pdata = (struct Parallelepiped *) obj;
	struct Rectangle *base = (struct Rectangle *) pdata->super.super;
	double volume = base->width * base->height * pdata->z;
	printf("Parallelepiped volume is %.2f\n", volume);
}

void Parallelepiped_moveTo(struct Shape *obj, int newx, int newy) {
	struct Parallelepiped *pdata = (struct Parallelepiped *) obj;
	rectangleFuncTable.super.moveTo((struct Shape *)pdata->super.super, newx, newy);
}

void Parallelepiped_destroy(struct Volume *obj) {
	struct Parallelepiped *pdata = (struct Parallelepiped *) obj;
	Shape_destroy((struct Shape *)pdata->super.super);
	Volume_destroy(obj);
	free(obj);
}

struct ParallelepipedFuncTable {
	struct VolumeFuncTable super;
} parallelepipedFuncTable = {
			{
			.printVolume = Parallelepiped_print_volume,
			.destructor_ = Parallelepiped_destroy
			}
};

struct Volume *Parallelepiped_init(struct Rectangle* rectangle, int initz)
{
	struct Parallelepiped *obj =
		(struct Parallelepiped *) malloc(sizeof(struct Parallelepiped));
	/*Define the function table for the Parallelepiped class */
		obj->super.funcTable = (struct VolumeFuncTable *) &parallelepipedFuncTable;
	obj->super.super = (struct Shape *)rectangle;
	obj->z = initz;
	return (struct Volume *) obj;
}


#define Shape_PRINTAREA(obj) (((struct Shape *) (obj))->funcTable->printArea((obj)))
#define Shape_MOVETO(obj, newx, newy)					\
	(((struct Shape *) (obj))->funcTable->moveTo((obj),(newx), (newy)))

#define Rectangle_SETWIDTH(obj, width)					\
	((struct RectangleFuncTable *) ((struct Shape *) (obj))->funcTable)->setWidth((obj), (width))

#define Shape_DESTROY(obj) (((struct Shape *) (obj)) -> funcTable->destructor_((obj)))

#define Volume_PRINTVOLUME(obj) (((struct Volume *) (obj))->funcTable->printVolume((obj)))
#define Volume_DESTROY(obj) (((struct Volume *) (obj))->funcTable->destructor_((obj)))


/*A function that uses a Shape polymorphically */
void handleShape(struct Shape *s) {
	Shape_MOVETO(s, 0, 0);
}

int main () {
	int i;
	struct Shape *shapes[2];
	struct Shape *r;

	/* Using shapes polymorphically */
	shapes[0] = Rectangle_init(20, 12, 123, 321);
	shapes[1] = Circle_init(21, 12, 2012);
	for (i = 0; i < 2; ++ i) {
		Shape_PRINTAREA(shapes[i]);
		handleShape(shapes[i]);
	}
	/* Accessing Rectangle specific data */
	r = Rectangle_init(1, 2, 3, 4);
	Rectangle_SETWIDTH(r, 5);
	Shape_PRINTAREA(r);
	Shape_DESTROY(r);
	for (i = 1; i >= 0; --i)
		Shape_DESTROY(shapes[i]);

	struct Volume *volume = Parallelepiped_init((struct Rectangle *) Rectangle_init(2, 2, 2, 2), 5);
	Volume_PRINTVOLUME(volume);
	handleShape((struct Shape *)((struct Parallelepiped *)volume)->super.super);
	Volume_DESTROY(volume);
}
