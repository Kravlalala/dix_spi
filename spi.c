/*
 * spi.c
 *
 *  Created on: May 31, 2017
 *      Author: ilya
 */

#include "spi.h"

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
