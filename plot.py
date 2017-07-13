import matplotlib as mpl
mpl.use('Agg')

import matplotlib.pyplot as plt
import sys

of = open(sys.argv[1],"r")
lines = of.readlines()
of.close()

rtt = []
btlbw = []
minrtt = []
maxbtlbw = []
gain = []
timev = []
modes = []
inflight = []
bdp = []
status = 0
n = 0
for line in lines:
	if line.find("ACK")!=-1 and len(line.split(" ")) > 10:
		# ACK:0.069054 MinRTT: 6.000000 RTT: 6.000000 MaxBtlBw: 579.255827 BtlBw: 579.255827 Mode: 1 Gain: 8.323225 Inflight: 1.000000
		timeun = float(line.split(" ")[0].split(":")[1])
		nowminrtt = float(line.split(" ")[2])
		nowrtt = float(line.split(" ")[4])
		nowbtlbw = float(line.split(" ")[8])/1024
		nowmaxbtlbw = float(line.split(" ")[6])/1024
		nowgain = float(line.split(" ")[12])
		nowinflight = float(line.split(" ")[14])
		nowstatus = int(line.split(" ")[10])
		bdp.append(nowbtlbw*nowrtt);
		inflight.append(nowinflight)
		if status!=nowstatus:
			modes.append(timeun)
			print str(timeun)+" STATUS "+str(nowstatus)
			status = nowstatus
		timev.append(timeun)
		gain.append(nowgain)
		rtt.append(nowrtt)
		btlbw.append(nowbtlbw)
		minrtt.append(nowminrtt)
		maxbtlbw.append(nowmaxbtlbw)
		n = n +1

plt.figure(figsize=(16,12))

ax = plt.subplot(221)
ax.plot(timev,rtt)
for mode in modes:
	ax.axvline(mode, c='grey', label='status')
ax.set_title("RTT")

ax = plt.subplot(222)
ax.text(timev[5],bdp[5], r'BDP', fontsize=10,color="red", verticalalignment='bottom', horizontalalignment='left')
ax.text(timev[10],inflight[10], r'INFLIGHT', fontsize=10,color="green", verticalalignment='bottom', horizontalalignment='right')
ax.plot(timev,bdp,c="red")
ax.plot(timev,inflight,c="green")
ylimv = max(max(bdp),max(inflight))*1.5
ax.set_ylim(0,ylimv)
for mode in modes:
	plt.axvline(mode, c='grey', label='status')
ax.set_title("BDP/INFLIGHT")

ax = plt.subplot(223)
ax.plot(timev,btlbw,c="green")
ax.plot(timev,maxbtlbw,c="red")
for mode in modes:
	plt.axvline(mode, c='grey', label='status')
ax.set_title("BANDWIDTH")
ax.set_ylabel("Kbytes")

ax = plt.subplot(224)
ax.plot(timev,gain)
for mode in modes:
	plt.axvline(mode, c='grey', label='status')
ax.set_title("GAIN")

plt.tight_layout()
plt.savefig('results.png')
plt.close()
