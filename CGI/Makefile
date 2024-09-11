# $Id$

include $(PWD)/Platform/$(PLATFORM).mk

OBJS = main.o cgi.o util.o version.o

.PHONY: all clean
.SUFFIXES: .c .o

all: keine.cgi

keine.cgi: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.cgi *.o
