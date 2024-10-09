#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "button.h"
#include "hex.h"
#include "led.h"

#define BASE_ADDR 0xFF200000
/*Register*/
#define BUTTON_OFFSET 0x50
#define LED_OFFSET 0x00
#define HEX_OFFSET_0_3 0x20
#define HEX_OFFSET_4_5 0x30

/*values*/
#define MIN 0
#define MAX 99
#define DELAY_US 10000 // 10 ms
#define LONG_PRESS_DELAY 100000 // 100 ms

#define LONG_PRESS_DURATION 1500 // 1.5sconds

struct {
	volatile uint8_t *base_addr;
	long page_size;
	int mem_fd;
} counter_ctl;

/// @brief Method to clear the hex display and the leds
void clear();

/// @brief Method to initialize the system
/// @return 0 if the initialization is successful, -1 otherwise
int initialize();

/// @brief Method to finish the system
void finish();

bool running = true; // Variable qui contrôle la boucle while

// Important: thanks to this function, the program will exit properly and the
// mem fd will be closed
void handle_sigint(int sig)
{
	printf("\nInterrupt signal caught\nProgram will now exit properly\n");
	running = false;
}

int main()
{
	/*Map the signal SIGINT to the function handle_sigint*/
	signal(SIGINT, handle_sigint);

	if (initialize() == -1) {
		printf("Error: initialization failed\n");
		return EXIT_FAILURE;
	}

	system("clear");
	printf("---------------------------------\n");
	printf("Welcome to the counter program\n");
	printf("---------------------------------\n\n");
	printf("Press KEY0 to increment and KEY1 to decrement\n");
	printf("Press and hold KEY0 or KEY1 to increment or decrement faster\n");
	printf("Press CTRL+C to exit properly\n\n");
	printf("---------------------------------\n\n");

	bool isPressed = false;
	uint8_t counter = MIN;
	uint8_t unit = 0;

	while (running) {
		int btn0 = read_button(0);
		int btn1 = read_button(1);

		if (btn0 == -1 || btn1 == -1) {
			printf("Error: cannot read button\n");
		}

		if ((btn0 && !isPressed) ||
		    (long_press(0, LONG_PRESS_DURATION) == 0)) {
			if (counter < MAX) {
				counter++;
				isPressed = true;
				printf("\rcounter: %d ", counter);
				fflush(stdout);
			}
		} else if ((btn1 && !isPressed) ||
			   (long_press(1, LONG_PRESS_DURATION) == 0)) {
			if (counter > MIN) {
				counter--;
				isPressed = true;
				printf("\rcounter: %d ", counter);
				fflush(stdout);
			}
		}

		if (!(btn0 || btn1)) {
			isPressed = false;
		}

		led_down(unit);
		unit = get_decimal_digit(counter, 0);
		display_decimal_number(counter);
		led_up(unit);

		// compute the delay if a button is long pressed
		long delay = (long_press(0, LONG_PRESS_DURATION) == 0 ||
			      long_press(1, LONG_PRESS_DURATION) == 0) ?
				     LONG_PRESS_DELAY :
				     DELAY_US;
		usleep(delay);
	}

	finish();
	return EXIT_SUCCESS;
}

void clear()
{
	clear_leds();
	clear_all_hex();
}

int initialize()
{
	printf("Initialization started\n");
	/*open memory file descriptor*/
	counter_ctl.mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (counter_ctl.mem_fd == -1) {
		perror("Error: cannot open /dev/mem");
		return -1;
	}

	/*get page size of system*/
	counter_ctl.page_size = sysconf(_SC_PAGESIZE);
	if (counter_ctl.page_size == -1) {
		perror("Error: cannot get page size");
		return -1;
	}

	/*Map /dev/mem to an address at a multiple of the page size on BASE_ADDR*/
	counter_ctl.base_addr = (uint8_t *)mmap(
		NULL, counter_ctl.page_size, PROT_READ | PROT_WRITE, MAP_SHARED,
		counter_ctl.mem_fd, BASE_ADDR & ~(counter_ctl.page_size - 1));
	if (counter_ctl.base_addr == MAP_FAILED) {
		perror("Error: memory mapping failed");
		close(counter_ctl.mem_fd);
		return -1;
	}

	/*Get pointers to the registers*/
	volatile uint32_t *btn_reg =
		(volatile uint32_t *)(counter_ctl.base_addr + BUTTON_OFFSET);
	volatile uint32_t *hex_reg_0_3 =
		(volatile uint32_t *)(counter_ctl.base_addr + HEX_OFFSET_0_3);
	volatile uint32_t *hex_reg_4_5 =
		(volatile uint32_t *)(counter_ctl.base_addr + HEX_OFFSET_4_5);
	volatile uint32_t *led_reg =
		(volatile uint32_t *)(counter_ctl.base_addr + LED_OFFSET);

	/*Initialize the system*/
	init_led(led_reg);
	init_hex(hex_reg_0_3, hex_reg_4_5);
	init_button(btn_reg);
	printf("Initialization completed\n");
	return 0;
}

void finish()
{
	clear();
	printf("Unmapping memory\n");
	munmap((uint32_t *)counter_ctl.base_addr, counter_ctl.page_size);
	printf("Closing memory file descriptor\n");
	close(counter_ctl.mem_fd);
	printf("Program successfully finished\n");
}
