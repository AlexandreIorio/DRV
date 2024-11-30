#include "linux/kfifo.h"
#include "music.h"
#include "playlist.h"
#include <linux/kernel.h>
#include <linux/string.h>

bool is_initilized_playlist(struct kfifo *playlist)
{
	bool is_init;

	is_init = (playlist != NULL) && kfifo_initialized(playlist);
	return is_init;
}

int get_music_from_string(struct music *music, const char *str)
{
	int ret;
	if (str == NULL) {
		pr_err("[%s]: String is NULL\n", LIB_NAME);
		return -EINVAL;
	}

	if (music == NULL) {
		pr_err("[%s]: Music is NULL\n", LIB_NAME);
		return -EINVAL;
	}

	ret = sscanf(str, "%" STR(NAME_SIZE) "s %" STR(ARTIST_SIZE) "s %d",
		     music->name, music->artist, &music->duration);
	if (ret != 3) {
		pr_err("[%s]: Failed to parse music\n", LIB_NAME);
		return -EINVAL;
	}
	pr_info("[%s]: Music parsed: Name='%s', Artist='%s', Duration=%d\n",
		LIB_NAME, music->name, music->artist, music->duration);

	return ret;
}

int set_music_to_playlist(struct kfifo *playlist, struct music *music)
{
	if (!is_initilized_playlist(playlist)) {
		pr_err("[%s]: Playlist is not initialized\n", LIB_NAME);
	}
	return 0;
}
