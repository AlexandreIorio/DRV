#include "music.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#define USAGE "Usage: %s <title> <artist> <duration in s>\n"
#define DEVICE_NAME "/dev/drivify"

int main(int argc, char *argv[])
{
	struct music music;
	char *ret_ptr;
	int fd;
	int nb_written;

	if (argc != 4) {
		perror("Invalid number of arguments.\n");
		perror(USAGE);
		return EXIT_FAILURE;
	}

	ret_ptr = strncpy(music.name, argv[1], NAME_SIZE);

	if (strlen(ret_ptr) == 0) {
		perror("The music name cannot be empty.\n");
		return EXIT_FAILURE;
	}

	ret_ptr = strncpy(music.artist, argv[2], ARTIST_SIZE);
	if (strlen(ret_ptr) == 0) {
		perror("The artist name cannot be empty.\n");
		return EXIT_FAILURE;
	}

	music.duration = atoi(argv[3]);
	if (music.duration <= 0) {
		perror("Duration must be greater than 0.\n");
		return EXIT_FAILURE;
	}

	fd = open(DEVICE_NAME, O_WRONLY);
	if (fd < 0) {
		perror("Failed to open device");
		return EXIT_FAILURE;
	}
	nb_written = write(fd, &music, sizeof(music));
	if (nb_written < 0) {
		perror("Failed to write to write on device, please check dmesg");
		close(fd);
		return EXIT_FAILURE;

	} else if (nb_written != sizeof(music)) {
		perror("Failed to write the whole music structure, please check dmesg");
		close(fd);
		return EXIT_FAILURE;
	}

	printf("Song added: [Title]: '%s' [Artiste]: '%s', [Duration]: %d seconds.\n",
	       music.name, music.artist, music.duration);

	close(fd);
	return 0;
}
