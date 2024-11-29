#ifndef MUSIC_H
#define MUSIC_H

#include <linux/init.h>

#define NAME_SIZE 25
#define ARTIST_SIZE 25
#define PLAYLIST_SIZE 16
#define LIB_NAME "music"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

struct music {
	char name[NAME_SIZE];
	char artist[ARTIST_SIZE];
	uint32_t duration;
};

struct playlist {
	struct music musics[PLAYLIST_SIZE];
	unsigned int playlist_index;
	unsigned int current_music_index;
};

///@brief get a music from a string
///@param str the string to parse
///@param music the music structure to fill
///@return 0 if no error or a negative error code
int get_music_from_string(struct music *music, const char *str);

///@brief get a string from a music that can be written to a virtual file
///@param buffer the buffer to write to
///@param music the music to convert
///@return the number of bytes written or a negative error code
int music_to_writtable_string(char *buffer, struct music *music);

///@brief set the music to the playlist
///@param playlist the playlist to set the music to
///@param music the music to set
///@return 0 if no error or a negative error code
int set_music_to_playlist(struct playlist *playlist, struct music *music);

#endif // MUSIC_H
