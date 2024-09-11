# $Id$

PWD = `pwd`
PLATFORM = generic

FLAGS = PWD=$(PWD) PLATFORM=$(PLATFORM)

.PHONY: all format clean ./CGI

./CGI::
	$(MAKE) -C $@ $(FLAGS)

format:
	clang-format -i --verbose `find . -name "*.c" -or -name "*.h"`

clean:
	$(MAKE) -C ./CGI $(FLAGS) clean
