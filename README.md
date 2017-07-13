# Google BBR implementation for ns-2

This is an implementation of Google's BBR TCP Congestion Control Algorithm for the ns-2 network simulator to be used on the [ICCRG TCP Evaluation Suite](https://riteproject.eu/resources/ietf-drafts/)

### How to install & use it

1. First, make sure your simulato setup works and compiles without applying this.
2. Now, you should install [the patch that enables pacing on ns2.34](http://jordan.auge.free.fr/misc)
3. Copy the code to `tcp/linux/src/bbr.c` under the ns source tree
4. Add an entry in the Makefile.in:
  ```
  OBJ_CC = \
  ...
  tcp/linux/src/bbr.o \
  ...
  ```
5. Recompile the simulator.
  ```
  configure
  make
  ```
  o
  ```
  install
  ```
6. Include the following lines in your tcl script to configure a BBR sender node:
  ```
  ...
  set tcp_sender [new Agent/TCP/Linux]
  $tcp_sender set timestamps_ true
  $tcp_sender select_ca bbr
  ...
  ```

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
