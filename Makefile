#
# Makefile for FlashProg-DE2-115 project
#

VERSION = 1.0

DIRS = fpga host

all:
		for i in $(DIRS) ; do \
		  $(MAKE) -C $$i all ; \
		done

clean:
		for i in $(DIRS) ; do \
		  $(MAKE) -C $$i clean ; \
		done
		rm -f *~

dist:		clean
		(cd .. ; \
		 tar --exclude-vcs -cvf \
		   flashprog-DE2-115-$(VERSION).tar \
		   flashprog-DE2-115/* ; \
		 gzip -f flashprog-DE2-115-$(VERSION).tar)
