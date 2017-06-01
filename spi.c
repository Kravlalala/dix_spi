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
	/*uint8_t default_tx[] = {
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
	 .bits_per_word = 8, };*/

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

}

void
dix_init (int fd) {
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
	tx[0] = 0x0d;
	tx[2] = 0x00;
	transfer (fd, tx, rx, 3);

	tx[0] = 0x0e;
	tx[2] = 0x07;
	transfer (fd, tx, rx, 3);

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

	/* Set up port A */
	tx[0] = 0x03;
	tx[2] = 0x29;
	transfer (fd, tx, rx, 3);

	tx[0] = 0x04;
	tx[2] = 0x05;
	transfer (fd, tx, rx, 3);

	/* Enable all functional blocks */
	tx[0] = 0x01;
	tx[2] = 0x32;
	transfer (fd, tx, rx, 3);

}
