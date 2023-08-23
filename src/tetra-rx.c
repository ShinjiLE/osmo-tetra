/* Test program for tetra burst synchronizer */

/* (C) 2011 by Harald Welte <laforge@gnumonks.org>
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/stat.h>

#include <osmocom/core/utils.h>
#include <osmocom/core/talloc.h>

#include "tetra_common.h"
#include "crypto/tetra_crypto.h"
#include <phy/tetra_burst.h>
#include <phy/tetra_burst_sync.h>
#include "tetra_gsmtap.h"

void *tetra_tall_ctx;

int main(int argc, char **argv)
{
	int fd;
	int opt;
	int pack_bits=0;
	struct tetra_rx_state *trs;
	struct tetra_mac_state *tms;

	/* Initialize tetra mac state and crypto state */
	tms = talloc_zero(tetra_tall_ctx, struct tetra_mac_state);
	tetra_mac_state_init(tms);
	tms->tcs = talloc_zero(NULL, struct tetra_crypto_state);
	tetra_crypto_state_init(tms->tcs);

	trs = talloc_zero(tetra_tall_ctx, struct tetra_rx_state);
	trs->burst_cb_priv = tms;

	while ((opt = getopt(argc, argv, "d:k:P")) != -1) {
		switch (opt) {
		case 'd':
			tms->dumpdir = strdup(optarg);
			break;
		case 'k':
			load_keystore(optarg);
			break;
		case 'P':
			pack_bits=1;
			break;
		default:
			fprintf(stderr, "Unknown option %c\n", opt);
		}
	}

	if (argc <= optind) {
		fprintf(stderr, "Usage: %s [-d DUMPDIR] <file_with_1_byte_per_bit>\n", argv[0]);
		exit(1);
	}

	fd = open(argv[optind], O_RDONLY);
	if (fd < 0) {
		perror("open");
		exit(2);
	}

	tetra_gsmtap_init("localhost", 0);

	#define BUFLEN 64

	while (1) {
		uint8_t buf[BUFLEN];
		int len;
		if(pack_bits)
		{
			uint8_t temp_buf[BUFLEN/8];
			int temp_len=0;
			len=0;
			temp_len = read(fd, temp_buf, sizeof(temp_buf));
			if (temp_len < 0) {
				perror("read");
				exit(1);
			} else if (temp_len == 0) {
				printf("EOF");
				break;
			}
			while(temp_len)
			{
				for(int i = 0;i<=7;i++)
				{
					buf[len] = ( temp_buf[ sizeof(temp_buf)-temp_len ] >> i ) & 0x01;
					len++;
				}
				temp_len--;
			}
		}
		else
		{
			len = read(fd, buf, sizeof(buf));
			if (len < 0) {
				perror("read");
				exit(1);
			} else if (len == 0) {
				printf("EOF");
				break;
			}
		}
		tetra_burst_sync_in(trs, buf, len);
	}

	free(tms->dumpdir);
	talloc_free(trs);
	talloc_free(tms->tcs);
	talloc_free(tms);

	exit(0);
}
