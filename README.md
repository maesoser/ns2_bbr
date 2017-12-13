# Google BBR implementation for ns-2

This is an implementation of Google's BBR TCP Congestion Control Algorithm for the ns-2 network simulator to be used on the [ICCRG TCP Evaluation Suite](https://riteproject.eu/resources/ietf-drafts/)

**This is an unfinished implementation** Contributions are welcome.

### TL;DR

This folder has to be next to (at the same level as ) ns-allinone-2.35.

```
make patch
make all
make plot
```

### How to install & use it

1. We assume your working ns-allinone-2.35 folder is next to the folder containing this repository, that is:
```
|- bbr
|   |-Makefile
|   |-tcp.h
|   |- (...)
|
|- ns-allinone-2.35
    |- ns-2.35
    |- include
    |- bin
    |- (...)
```

2. If it is the first time you install this, yo have to patch three important files:

- `Makefile.in`
- `tcp/tcp.h`
- `lib/tcl/ns-defaults.tcl`

You can apply these patches by doing:

``
make patch
```

3. After doing this, you need to call `make all` to copy the code and the `.tcl` script into ns2.35. This copy is separated from the patch rule because `tcp-bbr.cc` is completely original, so you can replace it without breaking anything (but bbr). So, from now on and after performing these steps, if you want to play with the code and change it, the only thing you need to do to test it is to perform again: `make all`.

4. In order to plot the wonderful graphs you see above, you can call `make plot`.

**Don't hesitate to take a look at the Makefile, it is very simple.**

### Python script

There is a python script which plots some debug data printed by the tcp-bbr code. Take a look at it.

![Editing](https://github.com/maesoser/ns2_bbr/raw/master/results.png)

### Acknowledgements

This code is based on [Google's proposed patch for the linux kernel](https://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/commit/?id=0f8782ea14974ce992618b55f0c041ef43ed0b78). It's specific ns-2 implementation is heavily influenced on the [Ledbat implementation for ns-2 simulator](http://perso.telecom-paristech.fr/~drossi/index.php?n=Software.LEDBATtele)

### Bibliography about BBR TCP

[Google proposed patch for linux Kernel](https://patchwork.ozlabs.org/patch/671069/)

[Webpage with info about BBR TCP](http://kb.pert.geant.net/PERTKB/BbrTcp)

[A Quick Look at BBR TCP](http://blog.cerowrt.org/post/bbrs_basic_beauty/)

[Article on ACM Queue](http://queue.acm.org/)

[BBR TCP Commito to Linux Kernel](https://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/commit/?id=0f8782ea14974ce992618b55f0c041ef43ed0b78)

[BBR Congestion Control](https://lwn.net/Articles/701165/)

[Comments about BBR Paper](https://blog.acolyer.org/2017/03/31/bbr-congestion-based-congestion-control/)

### Bibliography about NS-2 TCP Implementations

[ns2 Timers](http://telecom.dei.unipd.it/ns/miracle/nsmiracle-howto/node6.html)
[A mini-tutorial for TCP-Linux in NS-2](http://netlab.caltech.edu/projects/ns2tcplinux/ns2linux/tutorial/index.html)

[LEDBAT Implementation](http://perso.telecom-paristech.fr/~drossi/index.php?n=Software.LEDBATtele)

[ns2 Linux-TCP](http://netlab.caltech.edu/projects/ns2tcplinux/ns2linux/index.html)

[NS-2 Enhancement project](http://netlab.caltech.edu/projects/ns2tcplinux/)

[draft-cardwell-iccrg-bbr-congestion-control-00](https://tools.ietf.org/html/draft-cardwell-iccrg-bbr-congestion-control-00)

[draft-cheng-iccrg-delivery-rate-estimation-00](https://tools.ietf.org/html/draft-cheng-iccrg-delivery-rate-estimation-00)
