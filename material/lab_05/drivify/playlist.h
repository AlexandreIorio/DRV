#include <linux/init.h>
#include <linux/kfifo.h>
#include "music.h"
#define PLAYLIST_SIZE 16

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

/// @brief Check if the playlist is initialized
/// @param playlist The playlist to check
/// @return true if the playlist is initialized, false otherwise
bool is_initilized_playlist(struct kfifo *playlist);

/// @brief Set a music to a playlist
/// @param playlist The playlist to fill
/// @return 0 if no error
/// @param music The music to add
/// @note in this method a spinlock is used to protect the playlist, the irq will be saved and restored
int set_music_to_playlist(struct kfifo *playlist, struct music *music,
			  spinlock_t *playlist_lock);

/// @brief Get a music from a playlist
/// @param playlist The playlist to get the music from
/// @param music The music to fill
/// @return 0 if no error
/// @note in this method a spinlock is used to protect the playlist, the irq will be saved and restored
int get_music_from_playlist(struct kfifo *playlist, struct music *music,
			    spinlock_t *playlist_lock);
