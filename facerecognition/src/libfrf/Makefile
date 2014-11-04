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
SOURCES = fb.c fdb.c fpb.c validate.c
OBJECTS = fb.o fdb.o fpb.o validate.o

all: $(SOURCES)
ifeq ($(OS), Darwin)
	$(CC) -dynamiclib $(CFLAGS) $(SOURCES) $(EXTRALIBS) -o libfrf.dylib
	$(CP) libfrf.dylib $(LOCALLIB)
else
ifeq ($(findstring CYGWIN,$(OS)), CYGWIN)
	$(CC) $(CFLAGS) -c $(SOURCES)
	ar rs libfrf.a $(OBJECTS)
	ranlib libfrf.a
	$(CC) -shared -o libfrf.dll -Wl,--out-implib=libfrf.dll.a -Wl,--export-all-symbols -Wl,--enable-auto-import -Wl,--whole-archive libfrf.a -Wl,--no-whole-archive $(CFLAGS)
	$(CP) libfrf.a $(LOCALLIB)
	$(CP) libfrf.dll.a $(LOCALLIB)
	$(CP) libfrf.dll $(LOCALLIB)
else
	$(CC) -shared $(SOURCES) $(CFLAGS) -o libfrf.so
	$(CP) libfrf.so $(LOCALLIB)
endif
endif

clean:
	$(RM) $(DISPOSABLEFILES)
	$(RM) -r $(DISPOSABLEDIRS)
