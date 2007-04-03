
# This software was developed at the National Institute of Standards and
# Technology (NIST) by employees of the Federal Government in the course
# of their official duties. Pursuant to title 17 Section 105 of the
# United States Code, this software is not subject to copyright protection
# and is in the public domain. NIST assumes no responsibility  whatsoever for
# its use by other parties, and makes no guarantees, expressed or implied,
# about its quality, reliability, or any other characteristic.
#
# The FMR library is built and linked statically with the minexv binary
# so the MINEX-only checks can be compiled out of the library.
#
include ../common.mk
LIBFMRSRC = ../libfmr/fedb.c ../libfmr/fmd.c ../libfmr/fmr.c ../libfmr/fvmr.c
all: minexv.c $(LIBFMRSRC)
	$(CC) -DMINEX $(CFLAGS) $(LIBFMRSRC) minexv.c -o $(LOCALBIN)/minexv
	$(CP) minexv.1 $(LOCALMAN)

clean:
	$(RM) *.o
