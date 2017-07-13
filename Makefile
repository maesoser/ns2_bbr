
NSVER = 2.35
NSPATH = ../ns-allinone-$(NSVER)
DSTPATH = /ns-$(NSVER)/tcp/linux/src/

all: tcl

tcl:
	clear
	cp tcp.h $(NSPATH)/ns-$(NSVER)/tcp/
	cp tcp-bbr.cc $(NSPATH)/ns-$(NSVER)/tcp/
	cp bbr_test.tcl $(NSPATH)/ns-$(NSVER)/
	cd $(NSPATH)/ns-$(NSVER)/; make

stest:
	cd ../ns-allinone-2.35/ns-2.35/; ./ns bbr_test.tcl > outbbr.txt
	mv ./ns-allinone-2.35/ns-2.35/outbbr.txt outbbr.txt
	python plot.py
test:
	cd ../ns-allinone-2.35/ns-2.35/; ./ns bbr_test.tcl
