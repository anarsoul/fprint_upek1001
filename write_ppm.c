#include <stdio.h>
#include <stdlib.h>

#define WIDTH (216)
#define ROWS 2048

int main(int argc, char *argv[])
{
	int line = 0, x;
	FILE *in, *out;
	unsigned char buf[WIDTH];

	if (argc != 2) {
		printf("Usage: %s filename\n", argv[0]);
		return 0;
	}

	in = fopen(argv[1], "rb");
	//fseek(in, 2, SEEK_SET);
	out = fopen("finger.ppm", "wb");
	fprintf(out, "P2\n");
	fprintf(out, "%d 2048\n", WIDTH);
	fprintf(out, "255\n");

	while (!feof(in) && line < ROWS) {
		if (!fread(buf, WIDTH, 1, in))
			break;
		for (x = 0; x < WIDTH; x++) {
			fprintf(out, "%d ", buf[x]);
		}
		fprintf(out, "\n");
		line++;
	}
	fclose(out);
}
