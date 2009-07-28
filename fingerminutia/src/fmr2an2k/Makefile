#
# This software was developed at the National Institute of Standards and
# Technology (NIST) by employees of the Federal Government in the course
# of their official duties. Pursuant to title 17 Section 105 of the
# United States Code, this software is not subject to copyright protection
# and is in the public domain. NIST assumes no responsibility  whatsoever for
# its use by other parties, and makes no guarantees, expressed or implied,
# about its quality, reliability, or any other characteristic.
#
include ../common.mk
all: fmr2an2k.c
#
# OS-X has a libutil.dylib which clashes with NBIS' libutil.a, so the linking
# is done separately with the options set to find libtuil.a first.
#
ifeq ($(OS), Darwin)
	$(CC) -c $(CFLAGS) fmr2an2k.c -o fmr2an2k.o
	$(LD) -search_paths_first $(COMMONLIBOPT) -L../../lib /usr/lib/crt1.o fmr2an2k.o -lc -lfmr -lan2k -lioutil -lutil -o fmr2an2k
else
	$(CC) $(CFLAGS) fmr2an2k.c -lfmr -lan2k -lioutil -lutil -o fmr2an2k
endif
	$(CP) fmr2an2k $(LOCALBIN)
	$(CP) fmr2an2k.1 $(LOCALMAN)

clean:
	$(RM) fmr2an2k $(DISPOSABLEFILES)
	$(RM) -r $(DISPOSABLEDIRS)
