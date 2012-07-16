# This Makefile is hacked for this particular system.  It will need to be generalized
# in the future.

# We have learned that
#   pkg-config --cflags gio-2.0 glib-2.0 gio-unix-2.0
# returns
#   -pthread -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/gio-unix-2.0/  
# So we need to translate that to mzc syntax.  See MZC_CFLAGS

# We have also learned that
#    pkg-config --libs gio-2.0 glib-2.0 gio-unix-2.0
# returns
#    -lgio-2.0 -lgobject-2.0 -lglib-2.0 
# So we need to translate that to mzc syntax, too.  See MZC_LDLIBS

CFLAGS = -g -Wall -fPIC `pkg-config --cflags gio-2.0 glib-2.0 gio-unix-2.0` -I/usr/racket/include
LDLIBS = `pkg-config --libs gio-2.0 glib-2.0 gio-unix-2.0`

MZC_CFLAGS = ++ccf -g ++ccf -Wall ++ccf -pthread ++cppf -I/usr/include/glib-2.0 ++cppf -I/usr/lib/x86_64-linux-gnu/glib-2.0/include ++cppf -I/usr/include/gio-unix-2.0/  

MZC_LDLIBS = ++ldl gio-2.0 ++ldl gobject-2.0 ++ldl glib-2.0

default:rdbus.so

adding.so: adding_3m.o
	mzc --3m --ld adding.so adding_3m.o

adding_3m.o: adding.3m.c
	mzc --3m --cc adding.3m.c

adding.3m.c: adding.c
	mzc --xform adding.c

makeadder.so: makeadder_3m.o
	mzc --3m --ld makeadder.so makeadder_3m.o

makeadder_3m.o: makeadder.3m.c
	mzc --3m --cc makeadder.3m.c

makeadder.3m.c: makeadder.c
	mzc --xform makeadder.c

rdbus.so: rdbus_3m.o
	mzc $(MZC_CFLAGS) --3m --ld rdbus.so rdbus_3m.o

rdbus_3m.o: rdbus.3m.c
	mzc $(MZC_CFLAGS) --3m --cc rdbus.3m.c

rdbus.3m.c: rdbus.c
	mzc $(MZC_CFLAGS) --xform rdbus.c
