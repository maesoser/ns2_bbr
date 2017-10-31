# Google BBR implementation for ns-2

This is an implementation of Google's BBR TCP Congestion Control Algorithm for the ns-2 network simulator to be used on the [ICCRG TCP Evaluation Suite](https://riteproject.eu/resources/ietf-drafts/)

**This is an unfinished implementation** Contributions are welcome.

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

2. If your folder structure it's like that,you just need to invoke `make bbr` in order to get your files copied and compiled. **CAUTION**, the `make bbr` command overwrites an important header file on ns-2, `tcp.h.` If you previously modified this file, maybe you want to take a look at mine and copy just the modified lines.

3. Don't hesitate to take a look at the Makefile, it is very simple.

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
