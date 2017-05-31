/*
 * spi.h
 *
 *  Created on: May 31, 2017
 *      Author: ilya
 */

#ifndef SPI_H_
#define SPI_H_

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

void
set_device (int, uint32_t, uint8_t, uint32_t);

void print_frame();

void
transfer (int, uint8_t *, uint8_t *, size_t);

#endif /* SPI_H_ */
