#ifndef TETRA_UPPER_MAC_H
#define TETRA_UPPER_MAC_H

#include "tetra_prim.h"

#define REASSEMBLE_FRAGMENTS 1		/* Set to 0 to disable reassembly functionality */
#define FRAGSLOT_NR_SLOTS 5		/* Slot 0 is unused */

#define N203 6				/* Fragslot max age, see N.203 in the tetra docs, must be 4 multiframes or greater */
#define FRAGSLOT_MSGB_SIZE 8192
struct fragslot {
	bool active;			/* Set to 1 when fragslot holds a partially constructed message */
	uint32_t age;			/* Maintains the number of multiframes since the last fragment */
	int num_frags;			/* Maintains the number of fragments appended in the msgb */
	int length;			/* Maintains the number of bits appended in the msgb */
	bool encryption;		/* Set to true if the fragments were received encrypted */
	struct msgb *msgb;		/* Message buffer in which fragments are appended */
};

void upper_mac_init_fragslots();
int upper_mac_prim_recv(struct osmo_prim_hdr *op, void *priv);

#endif
