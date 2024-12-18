#include "linux/kfifo.h"
#include "music.h"
#include "playlist.h"
#include <linux/kernel.h>
#include <linux/string.h>

#define LIB_NAME "playlist"

bool is_initilized_playlist(struct kfifo *playlist)
{
	if (playlist == NULL) {
		pr_err("[%s]: Playlist is NULL\n", LIB_NAME);
		return false;
	}

	if (!kfifo_initialized(playlist)) {
		pr_err("[%s]: Playlist is not initialized\n", LIB_NAME);
		return false;
	}
	return true;
}

int set_music_to_playlist(struct kfifo *playlist, struct music *music,
			  spinlock_t *playlist_lock)
{
	int ret;

	if (!is_initilized_playlist(playlist)) {
		pr_err("[%s]: Playlist is not initialized\n", LIB_NAME);
		return -EINVAL;
	}

	if (music == NULL) {
		pr_err("[%s]: Music is NULL\n", LIB_NAME);
		return -EINVAL;
	}

	ret = kfifo_in_spinlocked(playlist, music, sizeof(struct music),
				  playlist_lock);

	if (ret != sizeof(struct music)) {
		pr_err("[%s]: The music could not be added to the playlist\n",
		       LIB_NAME);
		return -ENOSPC;
	}

	pr_info("[%s]: Music added to playlist: Title [%s] Artiste [%s] Duration [%d]\n",
		LIB_NAME, music->name, music->artist, music->duration);
	return 0;
}

int get_music_from_playlist(struct kfifo *playlist, struct music *music,
			    spinlock_t *playlist_lock)
{
	int ret;

	if (!is_initilized_playlist(playlist)) {
		pr_err("[%s]: Playlist is not initialized\n", LIB_NAME);
		return -EINVAL;
	}

	if (music == NULL) {
		pr_err("[%s]: Musici to filll is NULL\n", LIB_NAME);
		return -EINVAL;
	}

	ret = kfifo_out_spinlocked(playlist, music, sizeof(struct music),
				   playlist_lock);

	if (ret != sizeof(struct music)) {
		pr_err("[%s]: The music could not be retrieved from the playlist\n",
		       LIB_NAME);
		return -ENODATA;
	}

	pr_info("[%s]: Music retrieved from playlist\n", LIB_NAME);
	return 0;
}
