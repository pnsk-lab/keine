# $Id$

PWD = `pwd`
PLATFORM = generic

FLAGS = PWD=$(PWD) PLATFORM=$(PLATFORM)

.PHONY: all clean ./CGI

./CGI::
	$(MAKE) -C $@ $(FLAGS)

clean:
	$(MAKE) -C ./CGI $(FLAGS) clean
