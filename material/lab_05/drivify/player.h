#ifndef PLAYER_H
#define PLAYER_H

#include "drivify_shared_types.h"
#include <linux/kfifo.h>
#include "music.h"

/// @brief Add a new player to the playlist
/// @param player the player to initialize
/// @return 0 if no error
int initialize_player(struct player *player);

/// @brief Stop the player
/// @param player the player to stop
void stop_player(struct player *player);

int next_song(struct player *player);
int rewind_song(struct player *player);
int play_pause_song(struct player *player);
void refresh_player(struct player *player);
void get_current_song(struct player *player, struct music *current_song);
#endif // PLAYER_H
