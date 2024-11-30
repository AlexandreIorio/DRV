#ifndef MUSIC_H
#define MUSIC_H

#define NAME_SIZE 25
#define ARTIST_SIZE 25

struct music {
	char name[NAME_SIZE];
	char artist[ARTIST_SIZE];
	unsigned int duration;
};

#endif // MUSIC_H
