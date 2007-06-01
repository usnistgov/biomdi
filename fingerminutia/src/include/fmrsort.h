/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility  whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

/*
 * Define a structure that will be used to sort the minutiae.
 */
struct minutia_sort_data {
	FMD	*fmd;
	int	distance;	// linear distance between two points
	double	z;		// floating point distance
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

/* sort_fmd_by_random() modifies the input array by sorting the minutiae
 * randomly.
 * Parameters:
 *   fmds   : The array of pointers to the minutia data records.
 *   mcount : The number of minutiae.
 */
void sort_fmd_by_random(FMD **fmds, int mcount);
