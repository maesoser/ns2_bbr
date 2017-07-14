#
#	n0-|->ftp
#		 |----n2-------n3->Sink
#	n1-|
#
#Create a simulator object
set ns [new Simulator]

set tracefile [open bbrtraces.tr w]
$ns trace-all $tracefile

#Define different colors for data flows (for NAM)
$ns color 1 Blue
$ns color 2 Red

#Define a 'finish' procedure
proc finish {} {
        global ns nf tracefile
        $ns flush-trace
        #Close the NAM trace file
				close $tracefile
        #Execute NAM on the trace file
        exit 0
}

#Create four nodes
set n0 [$ns node]
set n1 [$ns node]
set n2 [$ns node]
set n3 [$ns node]

#Create links between the nodes
$ns duplex-link $n0 $n2 100Mb 20ms DropTail
$ns duplex-link $n1 $n2 100Mb 20ms DropTail
$ns duplex-link $n2 $n3 70Mb 60ms DropTail

#Set Queue Size of link (n0-n3) to 10
$ns queue-limit $n0 $n2 10
$ns queue-limit $n2 $n3 10

#Give node position (for NAM)
$ns duplex-link-op $n0 $n2 orient right-down
$ns duplex-link-op $n1 $n2 orient right-up
$ns duplex-link-op $n2 $n3 orient right

#Monitor the queue for link (n2-n3). (for NAM)
$ns duplex-link-op $n0 $n2 queuePos 0.5
$ns duplex-link-op $n2 $n3 queuePos 0.5

#Setup a TCP connection
set tcp [new Agent/TCP/Bbr]
$tcp set class_ 2
$tcp set fid_ 1
#$tcp set tcpTick_ 0.1              # timer granulatiry in sec (.1 is NONSTANDARD);
$ns attach-agent $n0 $tcp

#set sink [new Agent/TCPSink]
set sink [new Agent/TCPSink]
$ns attach-agent $n3 $sink
$ns connect $tcp $sink

# Let's trace some variables
$tcp attach $tracefile
$tcp tracevar rtt_
$tcp tracevar t_seqno_
$tcp tracevar seqno_

#Setup a FTP over TCP connection
set ftp [new Application/FTP]
$ftp attach-agent $tcp
$ftp set type_ FTP

$ns at 1.0 "$ftp start"
#$ns at 20.0 "$sink set interval_ 36ms"
#$ns at 60.0 "$sink set interval_ 46ms"
#$ns at 120.0 "$sink set interval_ 19ms"

$ns at 30.0 "$ftp stop"

#Detach tcp and sink agents (not really necessary)
$ns at 32 "$ns detach-agent $n0 $tcp ; $ns detach-agent $n3 $sink"

#Call the finish procedure after 5 seconds of simulation time
$ns at 33 "finish"

#puts "MinRTT = [$tcp set RTMin]"

#Run the simulation
$ns run
