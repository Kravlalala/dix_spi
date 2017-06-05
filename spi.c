/*
 * spi.c
 *
 *  Created on: May 31, 2017
 *      Author: ilya
 */

#include "spi.h"

/* Set devise controls
 * @fd - descriptor of spidev device.
 * @mode - spi mode (SPI_MODE0,1,...).
 * @bits - number bits in transferring word.
 * @speed - bus speed in Hz.
 */
void
set_device (int fd, uint32_t mode, uint8_t bits, uint32_t speed) {
	int ret;

	/* Set SPI mode */
	ret = ioctl (fd, SPI_IOC_WR_MODE32, &mode);
	if (ret == -1) {
		perror ("Couldn't set spi mode");
		abort ();
	}

	ret = ioctl (fd, SPI_IOC_RD_MODE32, &mode);
	if (ret == -1) {
		perror ("Couldn't get spi mode");
		abort ();
	}

	/* Set word length */
	ret = ioctl (fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1) {
		perror ("Couldn't set bits per word");
		abort ();
	}

	ret = ioctl (fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1) {
		perror ("Couldn't get bits per word");
		abort ();
	}

	/* Set max bus speed */
	ret = ioctl (fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1) {
		perror ("Couldn't set max speed");
		abort ();
	}

	ret = ioctl (fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1) {
		perror ("Couldn't get max speed");
		abort ();
	}

	printf ("spi mode: 0x%x\n", mode);
	printf ("bits per word: %d\n", bits);
	printf ("max speed: %d Hz (%d KHz)\n", speed, speed / 1000);
}

/* Output frame content in the terminal
 * @frame - pointer to the frame.
 * @frame_size - number of bytes per one frame.
 */
void
print_frame (const uint8_t *frame, const int frame_size) {
	for (int i = 0; i < frame_size; i++) {
		printf ("0x%x ", frame[i]);
	}
	printf ("\n");
}

/* SPI request in full duplex mode
 * @fd - descriptor of spidev device.
 * @tx - buffer for transmitted data.
 * @rx - buffer for recieved data.
 * @len - buffers size.
 */
void
transfer (int fd, uint8_t *tx, uint8_t *rx, size_t len) {
	int ret;

	struct spi_ioc_transfer tr = {
	    .tx_buf = (unsigned long) tx,
	    .rx_buf = (unsigned long) rx,
	    .len = len,
	    .delay_usecs = 0,
	    .speed_hz = 24000000,
	    .bits_per_word = 8, };

	ret = ioctl (fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1) {
		perror ("Couldn't send data");
		abort ();
	}

	/* Print transmitted frame */
	printf ("Transmit: ");
	print_frame (tx, len);

	/* Print recieved frame */
	printf ("Recieved: ");
	print_frame (rx, len);

}

/* Writing default data vector in DIX registers if command line arguments
 * hasn't been passed.
 * @fd - descriptor of spidev device.
 */
void
dix_init (int fd, const char *device) {
	/* Create buffers */
	uint8_t tx[3] = { [0 ... 2] = 0 };
	uint8_t rx[3] = { [0 ... 2] = 0 };

	/* Select page */
	tx[0] = 0x7f;
	tx[2] = 0x00;
	transfer (fd, tx, rx, 3);
	usleep (100);

	/* System reset */
	tx[0] = 0x01;
	tx[2] = 0x80;
	transfer (fd, tx, rx, 3);
	usleep (100);

	/* Power down all modules */
	tx[0] = 0x01;
	tx[2] = 0x00;
	transfer (fd, tx, rx, 3);
	usleep (100);

	/* DIR configure */
	if (strcmp (device, "/dev/spidev.1.3") == 0) {
		/* RX1 as input RXCKI as ref */
		tx[0] = 0x0d;
		tx[2] = 0x00;
		transfer (fd, tx, rx, 3);
	}
	if (strcmp (device, "/dev/spidev1.2") == 0) {
		/* Rx4 as input RXCKI as ref */
		tx[0] = 0x0d;
		tx[2] = 0x03;
		transfer (fd, tx, rx, 3);
	}

	/* Set divider after PLL2 to bypass */
	tx[0] = 0x0e;
	tx[2] = 0x10;
	transfer (fd, tx, rx, 3);

	/* Set up port X */
	if (strcmp (device, "/dev/spidev1.3") == 0) {
		/* Port A, I2S 24 bit, Master mode, DIR as source, mute output */
		tx[0] = 0x03;
		tx[2] = 0x69;
		transfer (fd, tx, rx, 3);

		/* Set divider=128 */
		tx[0] = 0x04;
		tx[2] = 0x08;
		transfer (fd, tx, rx, 3);
	}
	if (strcmp (device, "/dev/spidev1.2") == 0) {
		/* Port B, I2S 24 bit, Master mode, DIR as source, mute output */
		tx[0] = 0x05;
		tx[2] = 0x69;
		transfer (fd, tx, rx, 3);

		/* Set divider=128 */
		tx[0] = 0x06;
		tx[2] = 0x07;
		transfer (fd, tx, rx, 3);
	}

	/* Set up PLL1 */
	tx[0] = 0x0f;
	tx[2] = 0x22;
	transfer (fd, tx, rx, 3);

	tx[0] = 0x10;
	tx[2] = 0x00;
	transfer (fd, tx, rx, 3);

	tx[0] = 0x11;
	tx[2] = 0x00;
	transfer (fd, tx, rx, 3);

	if (strcmp (device, "/dev/spidev1.3") == 0) {
		/*Unmute Port A*/
		tx[0] = 0x03;
		tx[2] = 0x29;
		transfer (fd, tx, rx, 3);

		/* Enable all functional blocks except Port B and DIT */
		tx[0] = 0x01;
		tx[2] = 0x32;
		transfer (fd, tx, rx, 3);
	}
	if (strcmp (device, "/dev/spidev1.2") == 0) {
		/*Unmute Port B*/
		tx[0] = 0x05;
		tx[2] = 0x29;
		transfer (fd, tx, rx, 3);

		/* Enable all functional blocks except Port A and DIT */
		tx[0] = 0x01;
		tx[2] = 0x2A;
		transfer (fd, tx, rx, 3);
	}

}

/* System reset
 * @fd - descriptor of spidev device.
 */
void
dix_reset (int fd) {
	/* Create buffers */
	uint8_t tx[3] = { [0 ... 2] = 0 };
	uint8_t rx[3] = { [0 ... 2] = 0 };

	/* Reset sequence */
	tx[0] = 0x7f;
	tx[2] = 0x00;
	transfer (fd, tx, rx, 3);
	usleep (100);

	/* System reset */
	tx[0] = 0x01;
	tx[2] = 0x80;
	transfer (fd, tx, rx, 3);
	usleep (100);

	/* Power on all modules, except Port B and DIT */
	tx[0] = 0x01;
	tx[2] = 0x32;
	transfer (fd, tx, rx, 3);
	usleep (100);
}
