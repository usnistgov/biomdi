/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility  whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

/******************************************************************************/
/* This file contains the functions necessary to select a set of minutiae     */
/* using the Rectangular method.                                              */
/*                                                                            */
/******************************************************************************/
#include <sys/queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <fmr.h>
#include <m1io.h>

#include "fmrprune.h"

/* 
 * Sort by the rectangular method, as described above. Parameter
 * mcount is the total number of minutiae available, and is set
 * to the number of selected minutiae on return.
 */
void
sort_fmd_by_rectangular(FMD **fmds, int *mcount, int x, int y, int a, int b)
{
	int m, lcount;
	int Lx, Rx, Uy, Ly;
	struct minutia_sort_data *msds;

	/* Allocate an array to hold the sorting criteria for the FMDs */
	msds = (struct minutia_sort_data *)malloc(*mcount *
	    sizeof(struct minutia_sort_data));
	if (msds == NULL)
		ALLOC_ERR_EXIT("Sorting criteria array");
	for (m = 0; m < *mcount; m++)
		msds[m].fmd = fmds[m];

	/* Set the left and right-most x, upper and lower-most y */
	Lx = x;
	Uy = y;
	Rx = x + a;
	Ly = y + b;

	/* Check each minutia point for location inside the box */
	lcount = 0;
	for (m = 0; m < *mcount; m++) {
		if ((msds[m].fmd->x_coord >= Lx) &&
		    (msds[m].fmd->x_coord <= Rx) &&
		    (msds[m].fmd->y_coord >= Uy) &&
		    (msds[m].fmd->y_coord <= Ly)) {
			fmds[lcount] = msds[m].fmd;
			lcount++;
		}
	}
#ifdef DEBUG
	printf("Rectangle has coordinates of\n"
	    "UL(%d, %d), UR(%d, %d), LL(%d, %d), LR(%d, %d)\n"
	    " with %d minutiae contained within.\n",
	    Lx, Uy, Rx, Uy, Lx, Ly, Rx, Ly, lcount);
#endif

	*mcount = lcount;
	free(msds);
}
