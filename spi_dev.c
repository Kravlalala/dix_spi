/*
 * main.c
 *
 *  Created on: May 31, 2017
 *      Author: ilya
 */

#include "spi.h"

char *device = "/dev/spidev1.3";
int action_mask = 0;
char *param;

uint8_t reg_addr = 0;
uint8_t value = 0;

void
parse_opts (int, char **);

void
select_action (int, int, uint8_t *, uint8_t *);

int
main (int argc, char *argv[]) {

	int ret = 0;
	int fd;
	uint8_t *tx;
	uint8_t *rx;
	uint32_t mode = SPI_MODE_0;
	uint8_t bits = 8;
	uint32_t speed = 12000000;

	parse_opts (argc, argv);

	fd = open (device, O_RDWR);
	if (fd < 0)
		perror ("Couldn't open device");

	/* Set spidev ctrls */
	set_device (fd, mode, bits, speed);

	/* Read or Write data from/to register, depending on "action_mask" */
	select_action (action_mask, fd, tx, rx);
	close (fd);

	return ret;
}

/* Parsing passed command line arguments
 * @argc - number of passed arguments.
 * argv[] - array of passed arguments.
 *
 */
void
parse_opts (int argc, char *argv[]) {
	int ret = getopt (argc, argv, "r:w:v:d:p");

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
				action_mask |= 0x01;
				param = optarg;
				reg_addr = strtol (param, NULL, 16);
				//fprintf (stdout, "Reading value from reg %x\n", reg_addr);
				break;
			}

				/* If write parameter was detected */
			case 'w': {
				action_mask |= 0x2;
				param = optarg;
				reg_addr = strtol (param, NULL, 16);
				//fprintf (stdout, "Writing value to reg %x\n", reg_addr);
				break;
			}

				/* If writing value parameter was detected */
			case 'v': {
				param = optarg;
				value = strtol (param, NULL, 16);
				//fprintf (stdout, "Writing value %x\n", value);
				break;
			}

			case 'p': {
				param = optarg;
				action_mask |= 0x4;
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
		ret = getopt (argc, argv, "r:w:v:d:p");
	}
}

void
select_action (int action_mask, int fd, uint8_t *tx, uint8_t *rx) {
	int frame_size;

	switch (action_mask) {
		/* Default setup */
		case 0: {
			if (strcmp (device, "/dev/spidev1.3") == 0) {
				printf ("Setting spidev1.3 device to default state\n");
				dix_init (fd, "/dev/spidev1.3");
			}
			else if (strcmp (device, "/dev/spidev1.2") == 0) {
				printf ("Setting spidev1.2 to default state\n");
				dix_init (fd, "/dev/spidev1.2");
			}

			else
				fprintf (stderr,
				         "Unknown device! Please select device for default setup\n");
			break;
		}
			/* Read data from register */
		case 1: {
			char *current_dev = device;
			/* Allocate buffers */
			frame_size = 3;
			tx = malloc (sizeof(uint8_t) * 2);
			rx = malloc (sizeof(uint8_t) * 2);

			/* Create reading frame */
			tx[0] = reg_addr | 0x80;
			tx[1] = 0;
			tx[2] = 0;
			rx[0] = 0;
			rx[1] = 0;
			rx[2] = 0;

			/* Request data */
			transfer (fd, tx, rx, frame_size);

			/* Free buffers */
			free (tx);
			free (rx);
			break;
		}
			/* Write data to the register */
		case 2: {
			char *current_dev = device;
			frame_size = 3;
			tx = malloc (sizeof(uint8_t) * 2);
			rx = malloc (sizeof(uint8_t) * 2);

			/* Create reading frame */
			tx[0] = reg_addr & 0x7f;
			tx[1] = 0;
			tx[2] = value;
			rx[0] = 0;
			rx[1] = 0;
			rx[2] = 0;

			/* Send data */
			transfer (fd, tx, rx, frame_size);

			/* Free buffers */
			free (tx);
			free (rx);
			break;
		}
		case 4: {
			dix_reset (fd);
			break;
		}
		default:
			fprintf (stderr, "Unknown action\n");
			break;
	}
}
