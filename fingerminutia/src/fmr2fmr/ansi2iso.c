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
ansi2iso_fvmr(FVMR *ifvmr, FVMR *ofvmr, int *length)
{
	FMD **ifmds = NULL;
	FMD *ofmd;
	int m, mcount;
	double isotheta;
	int theta;

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

/* Convert an FVMR from ANSI to ISO Compact Card format.
 * Note this code does not remove minutiae (as required by 8 bit datatype) nor
 * implement the sort minutiae, (per, say, the cartesian y-x option in 19794-2).
 */
static int
ansi2isocc_fvmr(FVMR *ifvmr, FVMR *ofvmr, int *length,
    const unsigned short xres, const unsigned short yres)
{
	FMD **ifmds = NULL;
	FMD *ofmd;
	int mcount, m;

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

	for (m = 0; m < mcount; m++) {
		if (new_fmd(FMR_STD_ISO, &ofmd) != 0)
			ALLOC_ERR_RETURN("Output FMD");
		COPY_FMD(ifmds[m], ofmd);

		/* The ISO minutia record has 6 bits for the angle, so
		 * we have 64 possible values to represent 360 degrees.
		 */
		const int theta = 2 * (int)ifmds[m]->angle;
		const double isotheta = round((64.0 / 360.0) * (double)theta);
		ofmd->angle = (unsigned char)isotheta;

		const double x = (double)ifmds[m]->x_coord;
		const double y = (double)ifmds[m]->y_coord;

		/* millimeters, because INCITS 378 resolution 
		 * values are in pixels per centimeter */
		const double xmm = 10.0 * x / (double)xres;
		const double ymm = 10.0 * y / (double)yres;

		/* units of 0.1 pix per mm which is the compact
		 * card format's hardwired sampling freq */
 		const double xunits = xmm / 0.1;
 		const double yunits = ymm / 0.1;

		/* round the values - this is what would be
		 * stored in "typical" say 500 dpi operation */
 		ofmd->x_coord = (unsigned short)(0.5 + xunits);
 		ofmd->y_coord = (unsigned short)(0.5 + yunits);

		add_fmd_to_fvmr(ofmd, ofvmr);
		*length += FMD_DATA_LENGTH;
	}
	if (ifmds != NULL)
		free(ifmds);
	return (0);

err_out:
	if (ifmds != NULL)
		free(ifmds);
	return (-1);
}
