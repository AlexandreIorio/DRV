#include "music.h"
#include <linux/kernel.h>
#include <linux/string.h>
int get_music_from_string(struct music *music, const char *str)
{
	int ret;
	if (str == NULL) {
		pr_err("String is NULL\n");
		return -EINVAL;
	}

	if (music == NULL) {
		pr_err("Music is NULL\n");
		return -EINVAL;
	}

	ret = sscanf(str, "%" STR(NAME_SIZE) "s %" STR(ARTIST_SIZE) "s %d",
		     music->name, music->artist, &music->duration);
	if (ret != 3) {
		pr_err("Failed to parse music\n");
		return -EINVAL;
	}
	pr_info("Music parsed: Name='%s', Artist='%s', Duration=%d\n",
		music->name, music->artist, music->duration);

	return ret;
}

int set_music_to_playlist(struct playlist *playlist, struct music *music)
{
	pr_info("AAAAAAi\n");
	if (playlist == NULL) {
		pr_err("[%s]: Playlist is NULL\n", LIB_NAME);
		return -EINVAL;
	}

	if (music == NULL) {
		pr_err("[%s]: Music is NULL\n", LIB_NAME);
		return -EINVAL;
	}

	if (playlist->playlist_index >= PLAYLIST_SIZE) {
		pr_err("[%s]: Playlist is full\n", LIB_NAME);
		return -EINVAL;
	}
	pr_info("BBBBBBB\n");
	// if (!memcpy(&playlist->musics[playlist->playlist_index], music,
	// 	    sizeof(struct music))) {
	// 	pr_err("[%s]: Error copying music\n", LIB_NAME);
	// 	return -EINVAL;
	// };
	pr_info("CCCCCCC\n");
	playlist->playlist_index++;
	pr_info("[%s]: Music added to playlist\n", LIB_NAME);

	return 0;
}
