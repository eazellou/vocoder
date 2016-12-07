#ifndef BIT_CRUSH_H
#define BIT_CRUSH_H

DATA bitDepths[16] = {0xC000, 0xC000, 0xE000, 0xF000, 0xF800, 0xFC00, 0xFE00, 0xFF00, 0xFF80, 0xFFC0, 0xFFE0, 0xFFF0, 0xFFF8, 0xFFFC, 0xFFFE, 0xFFFF};

void processBitCrush(DATA *in, DATA *out, int bitDepth, int length) {
	DATA bitEnforcer = bitDepths[bitDepth - 1];
	int i;
	for (i = 0; i < length; i++) {
		out[i] = in[i] & bitEnforcer;
	}
}

#endif
