#ifndef MUSIC_H
#define MUSIC_H

#include <linux/init.h>

#define NAME_SIZE 25
#define ARTIST_SIZE 25

struct music {
	char name[NAME_SIZE];
	char artist[ARTIST_SIZE];
	uint32_t duration;
};

#endif // MUSIC_H
