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
	$(CC) $(CFLAGS) fmr2an2k.c -lfmr -lan2k -lioutil -lutil -o fmr2an2k
	$(CP) fmr2an2k $(LOCALBIN)
	$(CP) fmr2an2k.1 $(LOCALMAN)

clean:
	$(RM) fmr2an2k *.o
	$(RM) -r *.dSYM
	$(RM) .gdb_history
