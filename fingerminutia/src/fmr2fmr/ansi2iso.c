/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility  whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <sys/queue.h>
#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include <biomdimacro.h>
#include <fmr.h>

int
ansi2iso_fvmr_theta(FVMR *ifvmr, FVMR *ofvmr, int *length)
{
	FMD **ifmds = NULL;
	FMD *ofmd;
	int m, mcount;
	double isotheta;
	int theta;

	mcount = get_minutiae_count(ifvmr);
	if (mcount == 0)
		return (0);

	ifmds = (FMD **)malloc(mcount * sizeof(FMD *));
	if (ifmds == NULL)
		ALLOC_ERR_RETURN("FMD array");
	if (get_minutiae(ifvmr, ifmds) != mcount)
		ERR_OUT("getting FMDs from FVMR");

	*length = FVMR_HEADER_LENGTH;
	for (m = 0; m < mcount; m++) {
		/* The ISO minutia record uses all possible values for the
		 * angle, so we have 256 possible values to represent 360
		 * degrees.
		 */
		if (new_fmd(FMR_STD_ISO, &ofmd) != 0)
			ALLOC_ERR_RETURN("Output FMD");
		COPY_FMD(ifmds[m], ofmd);
		theta = 2 * (int)ifmds[m]->angle;
		isotheta = round((256.0 / 360.0) * (double)theta);
		ofmd->angle = (unsigned char)isotheta;
		add_fmd_to_fvmr(ofmd, ofvmr);
		*length += FMD_DATA_LENGTH;
	}

	free(ifmds);
	return (0);

err_out:
	if (ifmds != NULL)
		free(ifmds);
	return (-1);
}
