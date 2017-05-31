/*
 * main.c
 *
 *  Created on: May 31, 2017
 *      Author: ilya
 */

#include "spi.h"

char *device = "/dev/spidev1.3";
int action_mode = 0;
int ret = 0;
char *param;

uint8_t reg_addr = 0;
uint8_t value = 0;

static void
transfer (int fd, uint8_t const *tx, uint8_t const *rx, size_t len) {
	int ret;
	uint8_t default_tx[] = {
	    0xFF, 0xFF, 0xFF,0xFF,
	    0xFF, 0xFF, 0x40, 0x00,
	    0x00, 0x00, 0x00, 0x95,
	    0xFF, 0xFF, 0xFF, 0xFF,
	    0xFF, 0xFF, 0xFF, 0xFF,
	    0xFF, 0xFF, 0xFF, 0xFF,
	    0xFF, 0xFF, 0xFF, 0xFF,
	    0xFF, 0xFF, 0xF0, 0x0D,
	};

	uint8_t default_rx[32] = { 0, };

	struct spi_ioc_transfer tr = {
	    .tx_buf = (unsigned long) default_tx,
	    .rx_buf = (unsigned long) default_rx,
	    .len = sizeof(default_tx),
	    .delay_usecs = 0,
	    .speed_hz = 24000000,
	    .bits_per_word = 8, };

	ret = ioctl (fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1) {
		perror ("Couldn't send data");
		abort ();
	}
}

static void
parse_opts (int argc, char *argv[]) {
	int ret = getopt (argc, argv, "r:w:v:d:");

	/* Parsing parameters loop */
	while (ret != -1) {
		switch (ret) {

			/* If device selecting parameter was detected */
			case 'd': {
				device = optarg;
				fprintf (stdout, "Current master device is set on %s\n", device);
				break;
			}

				/* If read parameter was detected */
			case 'r': {
				action_mode |= 0x01;
				param = optarg;
				reg_addr = strtol (param, NULL, 16);
				fprintf (stdout, "Reading value from reg %x\n", reg_addr);
				break;
			}

				/* If write parameter was detected */
			case 'w': {
				action_mode |= 0x2;
				param = optarg;
				reg_addr = strtol (param, NULL, 16);
				fprintf (stdout, "Writing value to reg %x\n", reg_addr);
				break;
			}

				/* If writing value parameter was detected */
			case 'v': {
				param = optarg;
				value = strtol (param, NULL, 16);
				fprintf (stdout, "Writing value %x\n", value);
				break;
			}

				/* If unknown parameter was detected */
			case '?': {
				if (optopt == ret)
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint (optopt))
					fprintf (stderr, "Unknown option ' -%c'.\n", optopt);
				else
					fprintf (stderr, "Unknown option character '\\x%x'.\n", optopt);
				abort ();
			}

			default:
				break;
		}

		/* Get next param */
		param = 0;
		ret = getopt (argc, argv, "r:w:v:d:");
	}
}

int
main (int argc, char *argv[]) {

	int ret = 0;
	int fd;
	uint8_t *tx;
	uint8_t *rx;
	int size;
	uint32_t mode = SPI_MODE_0;
	uint8_t bits = 8;
	uint32_t speed = 24000000;

	parse_opts (argc, argv);

	fd = open (device, O_RDWR);
	if (fd < 0)
		perror ("Couldn't open device");

	set_device (fd, mode, bits, speed);

	transfer (fd, tx, rx, 0);
	close (fd);

	return ret;
}

