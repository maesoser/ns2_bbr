import matplotlib as mpl
mpl.use('Agg')

import matplotlib.pyplot as plt
import sys
from matplotlib.ticker import FormatStrFormatter

of = open(sys.argv[1],"r")
lines = of.readlines()
of.close()


bdp = []
timev = []
cwndgainv = []
rtpropv = []
btlbwv = []
pacinggainv = []
pacingratev = []
inflightv = []

modes = []
lastmode = ""
n = 0

for line in lines:
	if line.find("ACK")!=-1:
		line = line.split("\t")

		timestamp = float(line[0])
		BtlBw = float(line[2])
		RTprop = float(line[3])
		# SECS = RTprop/100
		pacing_gain = float(line[4])
		cwnd_gain = float(line[5])
		pacing_rate = float(line[6])
		inflight = float(line[7])
		mode = line[8].replace("\n","")

		bdp.append(BtlBw*RTprop);

		if lastmode!=mode:
			modes.append(timestamp)
			print str(timestamp)+" MODE: "+str(mode)
			lastmode = mode

		timev.append(timestamp)
		cwndgainv.append(cwnd_gain)
		rtpropv.append(RTprop)
		btlbwv.append(BtlBw)
		pacinggainv.append(pacing_gain)
		pacingratev.append(pacing_rate)
		inflightv.append(inflight)

		n = n +1

plt.figure(figsize=(24,16))

ax = plt.subplot(221)
ax.plot(timev,rtpropv)
for mode in modes:
	ax.axvline(mode, c='grey', label='status')
ax.set_title("RTProp")
ax.set_ylim(0,1)

ax = plt.subplot(222)
ax.set_title("BDP/INFLIGHT")
ax.text(timev[5],bdp[5], r'BDP', fontsize=10,color="red", verticalalignment='bottom', horizontalalignment='left')
ax.text(timev[10],inflightv[10], r'INFLIGHT', fontsize=10,color="green", verticalalignment='bottom', horizontalalignment='right')
ax.plot(timev,bdp,c="red")
ax.plot(timev,inflightv,c="green")
ylimv = max(max(bdp),max(inflightv))*1.5
ax.set_ylim(0,ylimv)
ax.yaxis.set_major_formatter(FormatStrFormatter('%.2f'))
for mode in modes:
	plt.axvline(mode, c='grey', label='status')

ax = plt.subplot(223)
ax.set_title("BTlBw")
ax.set_ylabel("MBytes")
ax.plot(timev,btlbwv,c="green")
ax.yaxis.set_major_formatter(FormatStrFormatter('%.2f'))
#ax.set_ylim(0,ylimv)
for mode in modes:
	plt.axvline(mode, c='grey', label='status')


ax = plt.subplot(224)
ax.set_title("GAIN")
ax.plot(timev,pacinggainv)
for mode in modes:
	plt.axvline(mode, c='grey', label='status')
ax.set_ylim(0.5,3)
plt.tight_layout()
plt.savefig('results.png')
plt.close()
