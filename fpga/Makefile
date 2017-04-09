#
# Makefile for DE2-115 Flash programmer
#

.PHONY:		all install clean

all:		build/output_files/flashprog.sof

install:	build/output_files/flashprog.sof

clean:
		mv build/flashprog.qpf .
		mv build/flashprog.qsf .
		if [ -f build/flashprog.qws ] ; then \
		  mv build/flashprog.qws . ; \
		fi
		if [ -f build/flashprog.srf ] ; then \
		  mv build/flashprog.srf . ; \
		fi
		if [ -f build/output_files/flashprog.sof ] ; then \
		  mv build/output_files/flashprog.sof . ; \
		fi
		if [ -f build/output_files/flashprog.cdf ] ; then \
		  mv build/output_files/flashprog.cdf . ; \
		fi
		rm -rf build/*
		mkdir build/output_files
		mv flashprog.qpf build
		mv flashprog.qsf build
		if [ -f flashprog.qws ] ; then \
		  mv flashprog.qws build ; \
		fi
		if [ -f flashprog.srf ] ; then \
		  mv flashprog.srf build ; \
		fi
		if [ -f flashprog.sof ] ; then \
		  mv flashprog.sof build/output_files ; \
		fi
		if [ -f flashprog.cdf ] ; then \
		  mv flashprog.cdf build/output_files ; \
		fi
		rm -f *~
