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
SOURCES = fir.c fivr.c validate.c
OBJECTS = fir.o fivr.o validate.o

all: $(SOURCES)
ifeq ($(OS), Darwin)
	$(CC) -c $(CFLAGS) $(SOURCES)
	$(CC) -dynamiclib $(CFLAGS) $(SOURCES) $(EXTRALIBS) -L$(BIOMDILIB) -lbiomdi -o libfir.dylib
	$(CP) libfir.dylib $(LOCALLIB)
else
ifeq ($(findstring CYGWIN,$(OS)), CYGWIN)
	$(CC) -c $(CFLAGS) $(SOURCES)
	ar rs libfir.a $(OBJECTS)
	ranlib libfir.a
	$(CC) $(CFLAGS) -shared -o libfir.dll -Wl,--out-implib=libfir.dll.a -Wl,--export-all-symbols -Wl,--enable-auto-import -Wl,--whole-archive libfir.a -Wl,--no-whole-archive -lbiomdi
	$(CP) libfir.a $(LOCALLIB)
	$(CP) libfir.dll.a $(LOCALLIB)
	$(CP) libfir.dll $(LOCALLIB)
else
	$(CC) $(CFLAGS) -shared $(SOURCES) -o libfir.so
	$(CP) libfir.so $(LOCALLIB)
endif
endif

clean:
	$(RM) $(DISPOSABLEFILES)
	$(RM) -r $(DISPOSABLEDIRS)
