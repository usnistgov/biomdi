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

/*
 * Convert an FVMR from ISO and ISO Normal Card formats to ANSI.
 * The finger minutiae data is copied, and the angle is converted to the
 * ANSI representation. The quality value is set to the unknown value.
 */
int
iso2ansi_fvmr(FVMR *ifvmr, FVMR *ofvmr, unsigned int *length)
{
	FMD **ifmds = NULL;
	FMD *ofmd;
	int m, mcount;
	double theta;
	double conversion_factor;

	COPY_FVMR(ifvmr, ofvmr);
	*length = FVMR_HEADER_LENGTH;
	mcount = get_minutiae_count(ifvmr);
	if (mcount == 0)
		return (0);

	ifmds = (FMD **)malloc(mcount * sizeof(FMD *));
	if (ifmds == NULL)
		ALLOC_ERR_RETURN("FMD array");
	if (get_minutiae(ifvmr, ifmds) != mcount)
		ERR_OUT("getting FMDs from FVMR");

	/* The ISO minutia record uses all possible values for the
	 * angle, so we have 256 possible values to represent 360
	 * degrees.
	 */
	conversion_factor = (360.0 / 256.0);
	for (m = 0; m < mcount; m++) {
		if (new_fmd(FMR_STD_ANSI, &ofmd) != 0)
			ALLOC_ERR_RETURN("Output FMD");
		COPY_FMD(ifmds[m], ofmd);
		theta = round(conversion_factor * (double)(ifmds[m]->angle));
		ofmd->angle = (unsigned char)(round(theta / 2));
		ofmd->quality = FMD_UNKNOWN_MINUTIA_QUALITY;
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

/*
 * Convert an FVMR from ISO Compact Card format to ANSI.
 */
int
isocc2ansi_fvmr(FVMR *ifvmr, FVMR *ofvmr, unsigned int *length,
    const unsigned short xres, const unsigned short yres)
{

	FMD **ifmds = NULL;
	FMD *ofmd;
	int m, mcount;
	double theta;
	double conversion_factor;
	double xcm, ycm;
	double xunits, yunits;

	COPY_FVMR(ifvmr, ofvmr);
	*length = FVMR_HEADER_LENGTH;
	mcount = get_minutiae_count(ifvmr);
	if (mcount == 0)
		return (0);

	ifmds = (FMD **)malloc(mcount * sizeof(FMD *));
	if (ifmds == NULL)
		ALLOC_ERR_RETURN("FMD array");
	if (get_minutiae(ifvmr, ifmds) != mcount)
		ERR_OUT("getting FMDs from FVMR");

	conversion_factor = (360.0 / 64.0);
	for (m = 0; m < mcount; m++) {
		if (new_fmd(FMR_STD_ANSI, &ofmd) != 0)
			ALLOC_ERR_RETURN("Output FMD");
		COPY_FMD(ifmds[m], ofmd);
		theta = conversion_factor * (double)(ifmds[m]->angle);
		theta = round(theta + 0.5);
		ofmd->angle = (unsigned char)(round(theta / 2));
		ofmd->quality = FMD_UNKNOWN_MINUTIA_QUALITY;

		/* ISO CC is 0.1 p/mm, so convert to 1 p/mm */
		xunits = (double)ifmds[m]->x_coord * 0.1;
		yunits = (double)ifmds[m]->y_coord * 0.1;

		/* Convert from p/mm to p/cm */
		// XXX The minutiae order needs to be considered here
		// XXX because the CC coord system wraps around
		xcm = (xunits * xres) / 10.0;
		ycm = (yunits * yres) / 10.0;

		ofmd->x_coord = (unsigned short)(0.5 + xcm);
		ofmd->y_coord = (unsigned short)(0.5 + ycm);

		add_fmd_to_fvmr(ofmd, ofvmr);
		*length += FMD_DATA_LENGTH;
	}

	free(ifmds);
	return (0);

err_out:
	return (-1);
}
