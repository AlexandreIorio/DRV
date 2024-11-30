#include <linux/init.h>
#include <linux/kfifo.h>
#include "music.h"
#define PLAYLIST_SIZE 16
#define LIB_NAME "playlist"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

/// @brief Check if the playlist is initialized
/// @param playlist The playlist to check
/// @return true if the playlist is initialized, false otherwise
bool is_initilized_playlist(struct kfifo *playlist);

/// @brief Get a music from a string
/// @param music The music to fill
/// @param str The string to parse
/// @return 0 if no error
int get_music_from_string(struct music *music, const char *str);

/// @brief Set a music to a playlist
/// @param playlist The playlist to fill
/// @param music The music to add
/// @return 0 if no error
int set_music_to_playlist(struct kfifo *playlist, struct music *music);
