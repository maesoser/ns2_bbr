NSVER = 2.35
NSPATH = ../ns-allinone-$(NSVER)
DSTPATH = /ns-$(NSVER)/tcp/linux/src/

all: update compile

patch:
	cp tcp.patch $(NSPATH)/ns-$(NSVER)/tcp/
	cd $(NSPATH)/ns-$(NSVER)/tcp/; patch tcp.h tcp.patch

	cp nsdefault.patch $(NSPATH)/ns-$(NSVER)/tcl/lib/
	cd $(NSPATH)/ns-$(NSVER)/tcl/lib/; patch ns-default.tcl nsdefault.patch 

	cp makefile.patch  $(NSPATH)/ns-$(NSVER)/
	cd $(NSPATH)/ns-$(NSVER)/; patch Makefile.in makefile.patch 
	
update:
	cp tcp-bbr.cc $(NSPATH)/ns-$(NSVER)/tcp/tcp-bbr.cc
	cp bbr_test.tcl $(NSPATH)/ns-$(NSVER)/

compile:
	cd $(NSPATH)/; ./install
	#cd $(NSPATH)/ns-$(NSVER)/; make

plot:
	cd ../ns-allinone-2.35/ns-2.35/; ./ns bbr_test.tcl > outbbr.txt
	mv ./ns-allinone-2.35/ns-2.35/outbbr.txt outbbr.txt
	python plot.py
test:
	cd ../ns-allinone-2.35/ns-2.35/; ./ns bbr_test.tcl
