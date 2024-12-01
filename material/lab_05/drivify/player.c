
#include "led.h"
#include "linux/container_of.h"
#include <linux/time.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include "player.h"
#include "hex.h"
#include "music.h"
#include "led.h"

#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/kfifo.h>

#define LIB_NAME "player"
#define TIMER_INTERVAL_NS (1 * 1000 * 1000 * 1000)

enum PLAYER_STATE {
	PLAYING, // The player is playing a song
	PAUSED, // The player is paused, the song can be resumed from where it was paused
};

enum PLAYER_COMMAND {
	NONE, // No command
	PLAY_PAUSE, // Playing or pausing the song
	REWIND, // Stop the song
	NEXT, // Play the next song
};

struct player_data {
	struct player *player;
	struct task_struct *player_thread;
	struct hrtimer player_timer;
	wait_queue_head_t wait_queue;
	int condition;
	enum PLAYER_STATE state;
	enum PLAYER_COMMAND command;
	struct music current_song;
	unsigned int current_duration;
};

static int run_player(void *player);
static void define_player_state(struct player_data *data);
static enum hrtimer_restart hrtimer_callback(struct hrtimer *timer);

static void play(struct player_data *data);
static void reset_current_song(struct player_data *data);
static void get_nb_songs(uint8_t *nb_songs, struct player_data *data);

int initialize_player(struct player *player)
{
	struct player_data *data;

	pr_info("[%s]: Initilizing\n", LIB_NAME);

	if (!player) {
		pr_err("[%s]: Failed to allocate memory for player\n",
		       LIB_NAME);
		return -ENOMEM;
	}

	if (!player->playlist) {
		pr_err("[%s]: Failed to allocate memory for playlist\n",
		       LIB_NAME);
		return -ENOMEM;
	}

	data = kmalloc(sizeof(struct player_data), GFP_KERNEL);

	init_waitqueue_head(&data->wait_queue);
	data->player_thread =
		kthread_run(run_player, (void *)data, "my_kthread");
	if (IS_ERR(data->player_thread)) {
		pr_err("[%s]: Failed to create kthread\n", LIB_NAME);
		return PTR_ERR(data->player_thread);
	}

	data->condition = 0;

	data->player = player;
	data->state = PAUSED;
	data->command = NONE;
	data->current_duration = 0;

	// used to access the player_data from stop method
	data->player->parent = data;

	// Initialiser le hrtimer
	hrtimer_init(&data->player_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	data->player_timer.function = hrtimer_callback;

	reset_current_song(data);
	clear_all_hex_0_3(player->hex_reg);
	clear_leds(player->led_reg);
	display_time_3_0(0, data->player->hex_reg);

	return 0;
}

void wake_up_player(struct player *player)
{
	struct player_data *data;
	printk(KERN_INFO "[%s]: Waking up player\n", LIB_NAME);
	data = (struct player_data *)player->parent;
	data->condition = 1;
	wake_up_interruptible(&data->wait_queue);
}

void stop_player(struct player *player)
{
	struct player_data *data;
	data = (struct player_data *)player->parent;

	pr_info("[%s]: Unloading hrtimer kthread\n", LIB_NAME);

	if (data->player_thread) {
		kthread_stop(data->player_thread);
	}

	hrtimer_cancel(&data->player_timer);
	clear_all_hex_0_3(player->hex_reg);
	clear_leds(player->led_reg);
	kfree(data);
}

static enum hrtimer_restart hrtimer_callback(struct hrtimer *timer)
{
	struct player_data *data;
	data = container_of(timer, struct player_data, player_timer);

	data->condition = 1;
	wake_up_interruptible(&data->wait_queue);
	hrtimer_forward_now(
		timer, ns_to_ktime(TIMER_INTERVAL_NS)); // Relancer le timer}
	return HRTIMER_RESTART;
}

static int run_player(void *player_data)
{
	uint8_t nb_songs;
	struct player_data *data;

	data = (struct player_data *)player_data;
	while (!kthread_should_stop()) {
		wait_event_interruptible(data->wait_queue,
					 data->condition ||
						 kthread_should_stop());
		define_player_state(data);
		if (data->state == PLAYING) {
			play(data);
		}

		get_nb_songs(&nb_songs, data);
		display_time_3_0(data->current_duration, data->player->hex_reg);
		clear_leds(data->player->led_reg);
		printk(KERN_INFO "[%s]: %d songs in playlist\n", LIB_NAME,
		       nb_songs);
		leds_up(nb_songs, data->player->led_reg);
		data->condition = 0; // Réinitialiser la condition
	}
	return 0;
}

static void play(struct player_data *data)
{
	if (data->current_duration >= data->current_song.duration) {
		data->current_duration = 0;
		if (kfifo_is_empty_spinlocked(data->player->playlist,
					      &data->player->playlist_lock)) {
			pr_info("[%s]: Playlist is empty\n", LIB_NAME);
			reset_current_song(data);
			data->command = PLAY_PAUSE;
			return;
		}
		data->command = NEXT;
		return;
	}

	data->current_duration++;
}

static void reset_current_song(struct player_data *data)
{
	data->current_song.duration = 0;
	data->current_song.name[0] = '\0';
	data->current_song.artist[0] = '\0';
}

static void get_nb_songs(uint8_t *nb_songs, struct player_data *data)
{
	if (data->command == NEXT) {
		return;
	} else if (data->state == PLAYING) {
		*nb_songs = kfifo_len(data->player->playlist) /
				    sizeof(struct music) +
			    1; // +1 pour le morceau en cours
	} else {
		*nb_songs = kfifo_len(data->player->playlist) /
			    sizeof(struct music);
	}
}

static void define_player_state(struct player_data *data)
{
	int ret;
	struct music next_music;
	switch (data->command) {
	case PLAY_PAUSE:
		switch (data->state) {
		case PLAYING:
			pr_info("[%s]: Pausing\n", LIB_NAME);
			data->state = PAUSED;
			hrtimer_cancel(&data->player_timer);
			break;
		case PAUSED:
			pr_info("[%s]: Playing\n", LIB_NAME);
			data->state = PLAYING;
			hrtimer_start(&data->player_timer,
				      ns_to_ktime(TIMER_INTERVAL_NS),
				      HRTIMER_MODE_REL);
			break;
		default:
			break;
		}
		break;
	case REWIND:
		data->current_duration = 0;

	case NEXT:
		ret = kfifo_out_spinlocked(data->player->playlist, &next_music,
					   sizeof(struct music),
					   &data->player->playlist_lock);
		if (ret == sizeof(struct music)) {
			pr_info("[%s]: Playing %s by %s\n", LIB_NAME,
				next_music.name, next_music.artist);
			data->current_duration = 0;
			memcpy(&data->current_song, &next_music,
			       sizeof(struct music));
		} else {
			pr_info("[%s]: Playlist is empty\n", LIB_NAME);
		}
		break;
	case NONE:
		break;
	default:
		pr_err("[%s]: Invalid command\n", LIB_NAME);
		break;
	}
	data->command = NONE;
}

int play_pause_song(struct player *player)
{
	struct player_data *data;

	if (!player) {
		pr_err("[%s]: Player is NULL\n", LIB_NAME);
		return -EINVAL;
	}

	data = (struct player_data *)player->parent;

	if (kfifo_is_empty_spinlocked(player->playlist,
				      &player->playlist_lock)) {
		pr_err("[%s]: Playlist is empty\n", LIB_NAME);
		return -EINVAL;
	}
	data->command = PLAY_PAUSE;
	data->condition = 1;

	return 0;
}

int rewind_song(struct player *player)
{
	struct player_data *data;

	if (!player) {
		pr_err("[%s]: Player is NULL\n", LIB_NAME);
		return -EINVAL;
	}

	data = (struct player_data *)player->parent;
	data->command = REWIND;
	data->condition = 1;
	return 0;
}

int next_song(struct player *player)
{
	struct player_data *data;
	if (!player) {
		pr_err("[%s]: Player is NULL\n", LIB_NAME);
		return -EINVAL;
	}
	data = (struct player_data *)player->parent;

	data->command = NEXT;
	data->condition = 1;
	return 0;
}
