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
 * Copy the minutiae from an ANSI FVMR to an ISO FVMR, altering
 * the angle value according to the ISO specification.
 * Parameters:
 *  ifvmr  - FVMR containing the finger minutiae data (FMD) records to copy
 *  ofvmr  - FVMR to contain the copied and modified FMDs
 *  length - Will contain the total length of the output FVMR
 *           (header plus length of all FMDs)
 */
int ansi2iso_fvmr(FVMR *ifvmr, FVMR *ofvmr, int *length);

/*
 * Copy the minutiae from an ANSI FVMR to an ISO Compact Card FVMR, altering
 * minutiae records:
 *   alter the angle value to reflect 6 bit precision
 *   alter the (x,y) coordinates to reflect 0.1 pix per millimeter units
 * Parameters:
 *  ifvmr  - FVMR containing the finger minutiae data (FMD) records to copy
 *  ofvmr  - FVMR to contain the copied and modified FMDs
 *  length - Will contain the total length of the output FVMR
 *           (header plus length of all FMDs)
 */
int ansi2isocc_fvmr(FVMR *ifvmr, FVMR *ofvmr, int *length);
