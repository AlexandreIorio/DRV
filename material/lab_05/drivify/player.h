#ifndef PLAYER_H
#define PLAYER_H

#include "drivify_shared_types.h"
#include <linux/kfifo.h>
#include "music.h"

/// @brief Add a new player to the playlist
/// @param player the player to initialize
/// @return 0 if no error
/// @note this method is called on the probe method then it is thread safe
int initialize_player(struct player *player);

/// @brief Stop the player
/// @param player the player to stop
void stop_player(struct player *player);

/// @brief Play the next song
/// @param player the player
/// @return 0 if no error
int next_song(struct player *player);

/// @brief Rewind the current song
/// @param player the player
/// @return 0 if no error
/// @note this method use a spinlock to protect the current song duration and to wake up the player thread
int rewind_song(struct player *player);

/// @brief Play or pause the current song
/// @param player the player
/// @return 0 if no error
/// @note this method use a spinlock to protect the current song duration and to wake up the player thread
int play_pause_song(struct player *player);

/// @brief Refresh the player
/// @param player the player
/// @note this method use a spinlock to protect the current song duration and to wake up the player thread
void refresh_player(struct player *player);

/// @brief Get the current song
/// @param player the player
/// @param music the buffer to store the song
/// @note in this method a spinlock is used to protect the current song when it memcopied, the irq will be saved and restored
void get_current_song(struct player *player, struct music *music);

/// @brief get the number of songs in the playlist of the player
/// @param player the player
/// @param nb_songs the buffer to store the number of songs
void get_nb_songs(struct player *player, uint8_t *nb_songs);

/// @brief get the total duration of the playlist of the player
/// @param player the player
/// @param total_duration the buffer to store the total duration
/// @note in this method a spinlock is used to protect the playlist when it memcopied, the irq will be saved and restored
void get_total_duration(struct player *player, uint32_t *total_duration);

/// @brief get the current duration of the player
/// @param player the player
/// @param current_duration the buffer to store the current duration
/// @return the number of seconds played since the beginning of the song -1 if no song is playing
int get_current_duration(struct player *player, uint32_t *current_duration);

/// @brief set the current duration of the player
/// @param player the player
/// @param current_duration the current duration
/// @return 0 if no error -1 if error
/// @note the current duration must be less than the duration of the current song
/// @note this use a spinlock to protect the current duration of the player
int set_current_duration(struct player *player, uint32_t current_duration);

/// @brief get the current state of the player
/// @param player the player
/// @return 0 if the player is paused, 1 if the player is playing and -1 if error
int get_player_state(struct player *player);

/// @brief play the current song
/// @param player the player
/// @note this method is thread safe because it will use thread safe methods
void do_play(struct player *player);

/// @brief pause the current song
/// @param player the player
/// @note this method is thread safe because it will use thread safe methods
void do_pause(struct player *player);

#endif // PLAYER_H
