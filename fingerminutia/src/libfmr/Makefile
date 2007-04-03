#
# This software was developed at the National Institute of Standards and
# Technology (NIST) by employees of the Federal Government in the course
# of their official duties. Pursuant to title 17 Section 105 of the
# United States Code, this software is not subject to copyright protection
# and is in the public domain. NIST assumes no responsibility  whatsoever for
# its use by other parties, and makes no guarantees, expressed or implied,
# about its quality, reliability, or any other characteristic.
#
# Set a variable so we can check the OS name; Mac OS-X (Darwin) uses a different
# form of linking libraries.
#
include ../common.mk
SOURCES = fmr.c fvmr.c fmd.c fedb.c

all: $(SOURCES)
ifeq ($(OS), Darwin)
	$(CC) -c $(CFLAGS) $(SOURCES)
	libtool -dynamic -o libfmr.dylib -lc *.o -lSystemStubs
	$(CP) libfmr.dylib $(LOCALLIB)
else
ifeq ($(findstring CYGWIN,$(OS)), CYGWIN)
	$(CC) $(CFLAGS) -c fmr.c
	$(CC) $(CFLAGS) -c fvmr.c
	$(CC) $(CFLAGS) -c fmd.c
	$(CC) $(CFLAGS) -c fedb.c
	ar rs libfmr.a fmr.o fvmr.o fmd.o fedb.o
	ranlib libfmr.a
	$(CC) -shared -o libfmr.dll -Wl,--out-implib=libfmr.dll.a -Wl,--export-all-symbols -Wl,--enable-auto-import -Wl,--whole-archive libfmr.a -Wl,--no-whole-archive
	$(CP) libfmr.a $(LOCALLIB)
	$(CP) libfmr.dll.a $(LOCALLIB)
	$(CP) libfmr.dll $(LOCALLIB)
else
	$(CC) $(CFLAGS) -shared $(SOURCES) -o libfmr.so
	$(CP) libfmr.so $(LOCALLIB)
endif
endif

clean:
	$(RM) *.dylib *.o *.so *.dll *.a
	$(RM) .gdb_history
