/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility  whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#define PRUNE_METHOD_POLAR		1
#define PRUNE_METHOD_ELLIPTICAL		2
#define PRUNE_METHOD_RANDOM		3
#define PRUNE_METHOD_RECTANGULAR	4

/*
 * Define a structure that will be used to sort the minutiae.
 */
struct minutia_sort_data {
	FMD	*fmd;
	int	distance;	// linear distance between two points
	double	z;		// elliptical distance
	int	rand;		// a random number associated with the record
};

/* sort_fmd_by_polar() modifies the input array by sorting the minutiae
 * using the Polar distance from the center of mass of the minutiae.
 * Minutiae with a shorter distance value are stored at lower array entries.
 * Parameters:
 *   fmds   : The array of pointers to the minutia data records.
 *   mcount : The number of minutiae.
 */
void sort_fmd_by_polar(FMD **fmds, int mcount);

/* sort_fmd_by_elliptical() modifies the input array by storing the minutiae
 * that fall within an ellipse centered on the center of mass of the
 * minutiae.
 * Parameters:
 *   fmds   : The array of pointers to the minutia data records.
 *   mcount : The number of minutiae in input; the actual number of
 *            minutiae that fall within the ellipse on output.
 *   a      : The semi-major axis of the ellipse.
 *   b      : The semi-minor axis of the ellipse.
 */
void sort_fmd_by_elliptical(FMD **fmds, int *mcount, int a, int b);

/* sort_fmd_by_random() modifies the input array by sorting the minutiae
 * randomly.
 * Parameters:
 *   fmds   : The array of pointers to the minutia data records.
 *   mcount : The number of minutiae.
 */
void sort_fmd_by_random(FMD **fmds, int mcount);

/* sort_fmd_by_rectangular() modifies the input array by storing the minutiae
 * that fall within an rectangle centered on the center of mass of the
 * minutiae.
 * Parameters:
 *   fmds   : The array of pointers to the minutia data records.
 *   mcount : On input, is the number of minutiae in the FMR. On output,
 *            returns the actual number of minutiae that fall within the
 *            rectangle.
 *   x      : The X coordinate of the upper left corner of the rectangle.
 *   y      : The Y coordinate of the upper left corner of the rectangle.
 *   a      : The width (X direction) of the rectangle.
 *   b      : The height (Y direction) of the rectangle.
 */
void sort_fmd_by_rectangle(FMD **fmds, int *mcount, int x, int y, int a, int b);
