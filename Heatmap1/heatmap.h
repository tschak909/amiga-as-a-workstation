#ifndef HEATMAP1_H
#define HEATMAP1_H

/**
 * Heatmap data structure
 */
struct Heatmap
{
	unsigned char data[16384];
	char name[80];
	char ts[80];
	char desc[80];
};

#endif /* HEATMAP1_H */
