#include <stdint.h>
#include <stdio.h>

static char hexdumpChar(char aInChar) {
	if ((aInChar <= 32) || (aInChar >= 127)) {
		return '.';
	} else {
		return aInChar;
	}
}

void hexdumpData(const unsigned char *aData, uint16_t aLength) {
	uint16_t i;
	fprintf(stderr, "%d bytes (0x%x) follow:\r\n", aLength, aLength);
	for (i = 0; i < aLength; i++) {
		if ((i % 16) == 0) {
			fprintf(stderr, "%4x   ", i);
		}
		fprintf(stderr, "%02x ", aData[i]);
		if (((i + 1) % 16) == 0) {
			uint16_t j;
			fprintf(stderr, "   ");
			for (j = i - 15; j <= i; j++) {
				fprintf(stderr, "%c", hexdumpChar(aData[j]));
			}
			fprintf(stderr, "\r\n");
		} else if (((i + 1) % 8) == 0) {
			fprintf(stderr, "   ");
		} else if (((i + 1) % 4) == 0) {
			fprintf(stderr, " ");
		}
	}
	if (aLength % 16) {
		uint16_t j;
		for (j = 0; j < 16 - (aLength % 16); j++) {
			fprintf(stderr, "   ");
		}
		if ((aLength % 16) < 4) {
			fprintf(stderr, "     ");
		} else if ((aLength % 16) < 8) {
			fprintf(stderr, "    ");
		} else if ((aLength % 16) < 12) {
			fprintf(stderr, " ");
		}
		fprintf(stderr, "   ");
		for (j = aLength - (aLength % 16); j < aLength; j++) {
			fprintf(stderr, "%c", hexdumpChar(aData[j]));
		}
		fprintf(stderr, "\r\n");
	}
}

void hexdumpDataLead(const char *aLead, const unsigned char *aData, uint16_t aLength) {
	fprintf(stderr, "%s ", aLead);
	hexdumpData(aData, aLength);
	fprintf(stderr, "\n");
}

#ifdef TEST_HEXDUMP
int main(int argc, char **argv) {
	int i;
	for (i = 0; i < 34; i++) {
		hexdumpData((unsigned char*)"Hallo du da. Wie geht es dir", i);
	}
	return 0;
}
#endif
