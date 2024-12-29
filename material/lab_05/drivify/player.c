
#include "led.h"
#include "linux/container_of.h"
#include "linux/hrtimer.h"
#include "linux/spinlock.h"
#include <linux/time.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include "player.h"
#include "hex.h"
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
#define LED_PLAYING 9
#define LEDS_SONGS 0x1F

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
	struct player *parent;
	struct task_struct *player_thread;
	struct hrtimer player_timer;
	wait_queue_head_t wait_queue;
	int condition;
	enum PLAYER_STATE state;
	enum PLAYER_COMMAND command;
	struct music current_song;
	unsigned int current_duration;
};

/// @brief The main loop of the player
/// @param player the player data
static void run_player(void *player);

/// @brief Define the player state
/// @param data the player data
/// @note this method is thread safe
static void define_player_state(struct player_data *data);

/// @brief Callback of the hrtimer
/// @param timer the timer
/// @return HRTIMER_RESTART
/// @note this method is called on a soft irq context when the timer expires it does not
/// need to be protected by a spinlock because all others spinlocks will save and restore the irq
static enum hrtimer_restart hrtimer_callback(struct hrtimer *timer);

/// @brief Wake up the player
/// @param data the player data
/// @note this method is thread safe
static void play(struct player_data *data);

/// @brief Pause the player
/// @param data the player data
static void reset_current_song(struct player_data *data);

/// @brief Display the number of songs on leds
/// @param data the player data
/// @note this method protect the access to the led register
static void display_nb_songs(struct player_data *data);

/// @brief Wake up the player
/// @param data the player data
static void wake_up_player(struct player_data *data);

static void reset_timer(struct player_data *data);

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

	data->parent = player;
	data->state = PAUSED;
	data->command = NONE;
	data->current_duration = 0;

	data->parent->data = data;

	// Initialize the timer
	hrtimer_init(&data->player_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	data->player_timer.function = hrtimer_callback;

	reset_current_song(data);
	clear_all_hex_0_3(player->hex_reg);
	clear_leds(player->led_reg);
	display_time_3_0(0, data->parent->hex_reg);

	return 0;
}

void stop_player(struct player *player)
{
	struct player_data *data;
	data = (struct player_data *)player->data;
	if (data->player_thread) {
		kthread_stop(data->player_thread);
	}

	hrtimer_cancel(&data->player_timer);
	clear_all_hex_0_3(player->hex_reg);
	clear_leds(player->led_reg);

	if (data != NULL) {
		kfree(data);
	}
}

void get_current_song(struct player *player, struct music *music_dest)
{
	struct player_data *data;
	unsigned long irq_flags;

	data = (struct player_data *)player->data;

	if (!player) {
		pr_err("[%s]: Player is NULL\n", LIB_NAME);
		return;
	}

	if (!data) {
		pr_err("[%s]: Data is NULL\n", LIB_NAME);
		return;
	}

	spin_lock_irqsave(&player->playlist_lock, irq_flags);
	memcpy(music_dest, &data->current_song, sizeof(struct music));
	spin_unlock_irqrestore(&player->playlist_lock, irq_flags);
}

void get_nb_songs(struct player *player, uint8_t *nb_songs)
{
	struct player_data *data;
	data = (struct player_data *)player->data;
	/// note that kfifo_len is thread safe
	if (data->current_song.duration != 0) {
		*nb_songs = kfifo_len(data->parent->playlist) /
				    sizeof(struct music) +
			    1; // +1 because the current song is in the count
	} else {
		*nb_songs = kfifo_len(data->parent->playlist) /
			    sizeof(struct music);
	}
}

void get_total_duration(struct player *player, uint32_t *total_duration)
{
	struct player_data *data;
	struct kfifo playlist;
	int nb_songs;
	struct music song;
	int ret;
	unsigned long irq_flags;

	data = (struct player_data *)player->data;

	spin_lock_irqsave(&player->playlist_lock, irq_flags);
	memcpy(&playlist, data->parent->playlist, sizeof(struct kfifo));
	spin_unlock_irqrestore(&player->playlist_lock, irq_flags);

	nb_songs = kfifo_len(&playlist) / sizeof(struct music);
	for (int i = 0; i < nb_songs; i++) {
		// this kfifo is local then it is not necessary to protect it
		ret = kfifo_out(&playlist, &song, sizeof(struct music));
		*total_duration += song.duration;
	}

	*total_duration += data->current_song.duration - data->current_duration;
}

int get_current_duration(struct player *player, uint32_t *current_duration)
{
	struct player_data *data;
	data = (struct player_data *)player->data;

	if (!player) {
		pr_err("[%s]: Player is NULL\n", LIB_NAME);
		return -EINVAL;
	}

	if (!data) {
		pr_err("[%s]: Data is NULL\n", LIB_NAME);
		return -EINVAL;
	}

	if (data->current_song.duration == 0) {
		*current_duration = -1;
		return 0;
	}

	*current_duration = data->current_duration;
	return 0;
}

int set_current_duration(struct player *player, uint32_t current_duration)
{
	struct player_data *data;
	unsigned long irq_flags;
	data = (struct player_data *)player->data;

	if (!player) {
		pr_err("[%s]: Player is NULL\n", LIB_NAME);
		return -1;
	}

	if (!data) {
		pr_err("[%s]: Data is NULL\n", LIB_NAME);
		return -1;
	}

	if (current_duration > data->current_song.duration) {
		pr_err("[%s]: Current duration is greater than the song duration\n",
		       LIB_NAME);
		return -1;
	}

	spin_lock_irqsave(&player->playlist_lock, irq_flags);
	data->current_duration = current_duration;
	data->condition = 1;
	wake_up_interruptible(&data->wait_queue);
	spin_unlock_irqrestore(&player->playlist_lock, irq_flags);
	reset_timer(data);
	return 0;
}

void do_play(struct player *player)
{
	struct player_data *data;

	data = (struct player_data *)player->data;

	if (data->state == PAUSED) {
		play_pause_song(player);
		return;
	}
}

void do_pause(struct player *player)
{
	struct player_data *data;
	data = (struct player_data *)player->data;

	if (data->state == PLAYING) {
		play_pause_song(player);
		return;
	}
}

int get_player_state(struct player *player)
{
	struct player_data *data;
	data = (struct player_data *)player->data;

	if (!player) {
		pr_err("[%s]: Player is NULL\n", LIB_NAME);
		return -EINVAL;
	}

	if (!data) {
		pr_err("[%s]: Data is NULL\n", LIB_NAME);
		return -EINVAL;
	}

	if (data->state == PLAYING) {
		return 1;
	} else if (data->state == PAUSED) {
		return 0;
	} else {
		return -1;
	}
}

static void wake_up_player(struct player_data *data)
{
	data->condition = 1;
	wake_up_interruptible(&data->wait_queue);
}

static void reset_timer(struct player_data *data)
{
	hrtimer_cancel(&data->player_timer);
	hrtimer_start(&data->player_timer, ns_to_ktime(TIMER_INTERVAL_NS),
		      HRTIMER_MODE_REL);
}

static enum hrtimer_restart hrtimer_callback(struct hrtimer *timer)
{
	struct player_data *data;
	data = container_of(timer, struct player_data, player_timer);

	data->condition = 1;
	wake_up_interruptible(&data->wait_queue);
	hrtimer_forward_now(
		timer, ns_to_ktime(TIMER_INTERVAL_NS)); // Restart the timer
	return HRTIMER_RESTART;
}

static void run_player(void *player_data)
{
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

		display_time_3_0(data->current_duration, data->parent->hex_reg);
		display_nb_songs(data);
		data->condition = 0; // Reset the condition
	}
}

static void play(struct player_data *data)
{
	unsigned long flags;

	/// This condition will lock the whole bloc of the if to be more efficient
	if (data->current_duration >= data->current_song.duration) {
		spin_lock_irqsave(&data->parent->playlist_lock, flags);
		data->current_duration = 0;
		if (kfifo_is_empty(data->parent->playlist)) {
			reset_current_song(data);
			data->command = PLAY_PAUSE;
			spin_unlock_irqrestore(&data->parent->playlist_lock,
					       flags);
			pr_info("[%s]: Playlist is empty\n", LIB_NAME);
			return;
		}
		data->command = NEXT;
		spin_unlock_irqrestore(&data->parent->playlist_lock, flags);
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

static void display_nb_songs(struct player_data *data)
{
	uint8_t nb_songs;
	unsigned long irq_flags;
	get_nb_songs(data->parent, &nb_songs);

	spin_lock_irqsave(&data->parent->playlist_lock, irq_flags);
	leds_down(LEDS_SONGS, data->parent->led_reg);
	leds_up(nb_songs & LEDS_SONGS, data->parent->led_reg);
	spin_unlock_irqrestore(&data->parent->playlist_lock, irq_flags);
}

static void define_player_state(struct player_data *data)
{
	int ret;
	unsigned long irq_flags;
	struct music next_music;

	/// note that if we lock a part of code, we use break and we unlock the code at the end.
	/// if we doesn't lock the code, we use return to exit the method
	switch (data->command) {
	case PLAY_PAUSE:
		switch (data->state) {
		case PLAYING:
			pr_info("[%s]: Pausing\n", LIB_NAME);
			data->state = PAUSED;
			spin_lock_irqsave(&data->parent->playlist_lock,
					  irq_flags);
			led_down(LED_PLAYING, data->parent->led_reg);
			hrtimer_cancel(&data->player_timer);
			break;
		case PAUSED:
			pr_info("[%s]: Playing :[%s]\n", LIB_NAME,
				data->current_song.name);

			data->state = PLAYING;
			spin_lock_irqsave(&data->parent->playlist_lock,
					  irq_flags);
			led_up(LED_PLAYING, data->parent->led_reg);
			hrtimer_start(&data->player_timer,
				      ns_to_ktime(TIMER_INTERVAL_NS),
				      HRTIMER_MODE_REL);
			break;
		default:
			break;
		}
		break;
	case REWIND:
		spin_lock_irqsave(&data->parent->playlist_lock, irq_flags);
		data->current_duration = 0;
		reset_timer(data);
		break;

	case NEXT:
		spin_lock_irqsave(&data->parent->playlist_lock, irq_flags);
		ret = kfifo_out(data->parent->playlist, &next_music,
				sizeof(struct music));
		if (ret == sizeof(struct music)) {
			data->current_duration = 0;
			memcpy(&data->current_song, &next_music,
			       sizeof(struct music));
			break;
		} else {
			pr_info("[%s]: Playlist is empty\n", LIB_NAME);
			break;
		}
	case NONE:
		//never spinlocked
		return;
	default:
		//never spinlocked
		pr_err("[%s]: Invalid command\n", LIB_NAME);
		return;
	}

	data->command = NONE;
	spin_unlock_irqrestore(&data->parent->playlist_lock, irq_flags);
}

int play_pause_song(struct player *player)
{
	struct player_data *data;
	unsigned long flags;

	if (!player) {
		pr_err("[%s]: Player is NULL\n", LIB_NAME);
		return -EINVAL;
	}

	data = (struct player_data *)player->data;

	spin_lock_irqsave(&player->playlist_lock, flags);
	data->command = PLAY_PAUSE;
	wake_up_player(data);
	spin_unlock_irqrestore(&player->playlist_lock, flags);

	return 0;
}

int rewind_song(struct player *player)
{
	struct player_data *data;
	unsigned long irq_flags;

	if (!player) {
		pr_err("[%s]: Player is NULL\n", LIB_NAME);
		return -EINVAL;
	}

	data = (struct player_data *)player->data;
	spin_lock_irqsave(&player->playlist_lock, irq_flags);
	data->command = REWIND;
	wake_up_player(data);
	spin_unlock_irqrestore(&player->playlist_lock, irq_flags);

	return 0;
}

int next_song(struct player *player)
{
	unsigned long irq_flags;
	struct player_data *data;

	if (!player) {
		pr_err("[%s]: Player is NULL\n", LIB_NAME);
		return -EINVAL;
	}
	data = (struct player_data *)player->data;

	/// This part protect a bloc of code to be more efficient
	spin_lock_irqsave(&player->playlist_lock, irq_flags);
	data->command = NEXT;
	if (!kfifo_is_empty(player->playlist)) {
		wake_up_player(data);
	}
	spin_unlock_irqrestore(&player->playlist_lock, irq_flags);
	reset_timer(data);
	return 0;
}

void refresh_player(struct player *player)
{
	struct player_data *data;
	unsigned long irq_flags;

	if (!player) {
		pr_err("[%s]: Player is NULL\n", LIB_NAME);
		return;
	}
	data = (struct player_data *)player->data;
	if (data->state == PAUSED) {
		spin_lock_irqsave(&player->playlist_lock, irq_flags);
		wake_up_player(data);
		spin_unlock_irqrestore(&player->playlist_lock, irq_flags);
	}
}
