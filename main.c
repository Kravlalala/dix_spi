/*
 * opts.c
 *
 *  Created on: May 30, 2017
 *      Author: ilya
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

int
main (int argc, char **argv) {
	int opt_flag = 0;
	int ret = 0;
	char *param;
	int reg_addr;
	int value;
	opterr = 0;

	/* Get first param */
	ret = getopt (argc, argv, "r:w:v:");

	/* Parsing parameters loop */
	while (ret != -1) {
		switch (ret) {

			/* If read parameter was detected */
			case 'r': {
				opt_flag |= 0x01;
				param = optarg;
				reg_addr = strtol (param, NULL, 16);
				fprintf (stdout, "Reading value from reg %x\n", reg_addr);
				break;
			}

				/* If write parameter was detected */
			case 'w': {
				opt_flag |= 0x2;
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
				else if (isprint(optopt))
					fprintf (stderr, "Unknown option ' -%c'.\n", optopt);
				else
					fprintf (stderr, "Unknown option character '\\x%x'.\n", optopt);
				return 1;
			}

			default:
				break;
		}
		/* Get next param */
		param = 0;
		ret = getopt (argc, argv, "r:w:v:");

	}

	return 0;
}

