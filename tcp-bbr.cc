/*
 * tcp-bbr.cc
 * Copyright (C) 2017 by Carlos III University of Madrid
 *
 */


#ifndef lint
static const char rcsid[] =
"@(#) $Header: /cvsroot/nsnam/ns-2/tcp/tcp-bbr.cc,v 0.01 2017/08/25 18:58:12 Sergio Maeso $ (UC3M)";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "ip.h"
#include "tcp.h"
#include "flags.h"

#define MIN(x, y) ((x)<(y) ? (x) : (y))
#define MAX(x, y) ((x)>(y) ? (x) : (y))

#define BBR_STARTUP 0x01
#define BBR_DRAIN	0x02
#define BBR_PROBE_BW	0x03
#define BBR_PROBE_RTT 0x04

#define BBR_PROBE_BW_TIME 10 // seconds
#define BBR_PROBE_RTT_TIME 0.2	// seconds

#define BBR_GROWTH_TARGET 1.25
#define BBR_N_STARTUP_RTT 3

#define bbr_mode_CHANGE 0x32

static class BbrTcpClass : public TclClass {
	public:
		BbrTcpClass() : TclClass("Agent/TCP/Bbr") {}
		TclObject* create(int, const char*const*) {
			return (new BbrTcpAgent());
		}
} class_Bbr;

/*
	Función de expiración del timer de pacing que se encarga de enviar
	periodicamente los paquetes.
*/
void PacingTimer::expire(Event *e){
	a_->timeout(TCP_TIMER_DELSND);
}

/*
	Función de expiración del timer de cambio de modo entre PROBE_BW y PROBE_RTT
*/
void ModeTimer::expire(Event *e){
	a_->timeout(bbr_mode_CHANGE);
}

BbrTcpAgent::BbrTcpAgent() : TcpAgent(), bbrpactimer(this), bbrmodetmr(this){
	BbrInit();
}

BbrTcpAgent::~BbrTcpAgent(){
}

void BbrTcpAgent::delay_bind_init_all(){
	TcpAgent::delay_bind_init_all();
  reset();
}

void BbrTcpAgent::UpdateRTprop(int rtt){
	rtprop_expired = Now() > RTprop_stamp + RTpropFilterLen;
	if ( (rtt >= 0) && (rtt <= RTprop || rtprop_expired) ){
		RTprop = rtt;
		RTprop_stamp = Now();
	}
}

void BbrTcpAgent::InitPacingRate(){
       double nominal_bandwidth = wnd_init_ / ((int)t_srtt_ ? (int)t_srtt_ : 1);
       pacing_rate =  pacing_gain * nominal_bandwidth;
}

void BbrTcpAgent::SetPacingRateWithGain( double pacing_gain){
	double rate = pacing_gain * BtlBw;
	if ( filled_pipe || (rate > pacing_rate) ){
		pacing_rate = rate;
	}
}

void BbrTcpAgent::ModulateCwndForProbeRTT(){
	if (bbr_mode  == BBR_PROBE_RTT)
		cwnd_ = MIN((double)cwnd_, MinPipeCwnd);
}

void BbrTcpAgent::UpdateTargetCwnd(){
	target_cwnd = BBRInflight(cwnd_gain);
}

void BbrTcpAgent::SetSendQuantum(){
  if (pacing_rate < 1200000)
    send_quantum = 1 * size_;
  else if (pacing_rate < 2400000)
    send_quantum  = 2 * size_;
  else
    send_quantum  = MIN(pacing_rate, 64000);
}

double BbrTcpAgent::BBRInflight(double gain){
	if (RTprop == -1)
		return wnd_init_; // no valid RTT samples yet
	int quanta = 3*send_quantum;
	double estimated_bdp = BtlBw * RTprop;
	return gain * estimated_bdp + quanta;
}

void BbrTcpAgent::UpdateBtlBw(double bw){
		bool BtlBw_expired = BtlBw_stamp > BtlBwFilterLen;
		if ( (bw >= 0) && (bw >= BtlBw || BtlBw_expired) ){
			BtlBw = bw;
			BtlBw_stamp = 0;
		}
		BtlBw_stamp ++;
}

void BbrTcpAgent::CheckFullPipe(){
	if(filled_pipe)
		return;  // no need to check for a full pipe now
	double target = bandwidth_at_last_round_ * BBR_GROWTH_TARGET;

	if (BtlBw >= target){  // BtlBw still growing?
		bandwidth_at_last_round_ = BtlBw;    // record new baseline level
		rounds_without_bandwidth_gain_ = 0;
		return;
	}
	rounds_without_bandwidth_gain_++; // another round w/o much growth
	if (rounds_without_bandwidth_gain_ >= BBR_N_STARTUP_RTT){
		filled_pipe = true;
	}
}

void BbrTcpAgent::ModulateCwndForRecovery(){
	if (packets_lost > 0)
		cwnd_ = MAX(cwnd_ - packets_lost, 1);
	if (packet_conservation)
		cwnd_ = MAX((double)cwnd_, packets_in_flight() + packets_delivered);
}

int BbrTcpAgent::packets_in_flight(){
	return t_seqno_;
}

void BbrTcpAgent::SetCwnd(){
	UpdateTargetCwnd();
	ModulateCwndForRecovery();

	if (! packet_conservation) {
		if (filled_pipe){
			cwnd_ = MIN((double)cwnd_ + packets_delivered, target_cwnd);
		}
		else if (cwnd_ < target_cwnd || delivered < wnd_init_){
			cwnd_ = cwnd_ + packets_delivered;
		}
		cwnd_ = MAX((double)cwnd_, MinPipeCwnd);
	}

	ModulateCwndForProbeRTT();
}

double BbrTcpAgent::getProbeBWGain(){
	double sol = 1;
	if(bbr_pacing_i==0) sol = bbr_highpacing_gain;
	if(bbr_pacing_i==1) sol = bbr_lowpacing_gain;

	bbr_pacing_i += (bbr_pacing_i== bbr_recoverypacing_rounds+2) ? 0 : 1;
	return sol;

}

int BbrTcpAgent::delay_bind_dispatch(const char *varName, const char *localName,TclObject *tracer){
	/* init vegas var */
				if (delay_bind(varName, localName, "RTprop", &RTprop, tracer))
		return TCL_OK;
				if (delay_bind(varName, localName, "BtlBw", &BtlBw, tracer))
		return TCL_OK;
				if (delay_bind(varName, localName, "bbr_mode", &bbr_mode, tracer))
		return TCL_OK;
  return TcpAgent::delay_bind_dispatch(varName, localName, tracer);
}

void BbrTcpAgent::reset(){
	BbrInit();

	TcpAgent::reset();
}

void BbrTcpAgent::EnterDrain(){
	bbr_mode = BBR_DRAIN;
	pacing_gain = bbr_drain_gain;  // pace slowly
	cwnd_gain = bbr_high_gain;    // maintain cwnd
}


void BbrTcpAgent::EnterProbeBW(){
		bbr_mode = BBR_PROBE_BW;
       pacing_gain = 1;
       cwnd_gain = 2;
       cycle_index = bbr_recoverypacing_rounds - 1 - rand()/6;
       AdvanceCyclePhase();
}

void BbrTcpAgent::AdvanceCyclePhase(){
	double pacing_gain_cycle[8] = {5/4, 3/4, 1, 1, 1, 1, 1, 1};
	cycle_stamp = Now();
	cycle_index = (cycle_index + 1) % bbr_recoverypacing_rounds;
	pacing_gain = pacing_gain_cycle[cycle_index];
}

void BbrTcpAgent::CheckDrain(){
	if (bbr_mode == BBR_STARTUP && filled_pipe)
		EnterDrain();
	if ( (bbr_mode == BBR_DRAIN) && (packets_in_flight() <= BBRInflight(1.0)) )
		EnterProbeBW();  // we estimate queue is drained
}

/*
   Here, "prior_inflight" is the amount of data that was in flight
   before processing this ACK.
 */
bool BbrTcpAgent::IsNextCyclePhase(){
	bool is_full_length = (Now() - cycle_stamp) > RTprop;

	if (pacing_gain == 1){
		return is_full_length;
	}
	if (pacing_gain > 1){
		return is_full_length && (
				packets_lost > 0 ||
				(prior_inflight >= BBRInflight(pacing_gain))
				);
	}
	else{  //  (BBR.pacing_gain < 1)
	 return (is_full_length || (prior_inflight <= BBRInflight(1)));
	}
}

void BbrTcpAgent::CheckCyclePhase(){
	if( (bbr_mode == BBR_PROBE_BW) && IsNextCyclePhase())
	 AdvanceCyclePhase();
}

void BbrTcpAgent::CheckProbeRTT(){
       if (bbr_mode != BBR_PROBE_RTT && rtprop_expired && !idle_restart)
         EnterProbeRTT();
         SaveCwnd();
         probe_rtt_done_stamp = 0;
       if (bbr_mode != BBR_PROBE_RTT)
         HandleProbeRTT();
       idle_restart = false;
}

void BbrTcpAgent::EnterProbeRTT(){
	bbr_mode = BBR_PROBE_RTT;
	pacing_gain = 1;
	cwnd_gain = 1;
}

void BbrTcpAgent::HandleProbeRTT(){
	/* Ignore low rate samples during ProbeRTT: */
	int app_limited = (packets_delivered + packets_in_flight()) ? : 1;
	if (probe_rtt_done_stamp == 0 && packets_in_flight <= BBRMinPipeCwnd){
		probe_rtt_done_stamp = Now() + ProbeRTTDuration;
		probe_rtt_round_done = false;
		next_round_delivered = delivered;
	}
		else if (probe_rtt_done_stamp != 0){
		if (round_start){
			probe_rtt_round_done = true
		}
		if (probe_rtt_round_done && Now() > probe_rtt_done_stamp){
			rtprop_stamp = Now()
			BBRRestoreCwnd()
			BBRExitProbeRTT()
		}
	}
}

void BbrTcpAgent::ExitProbeRTT(){
       if (filled_pipe)	EnterProbeBW();
       else 	EnterStartup();
}

/*

	[X]  BBRUpdateBtlBw()
	[X]  BBRCheckCyclePhase()  --> prior_inflight
	[X]  BBRCheckFullPipe()
	[X]  BBRCheckDrain()
	[X]  BBRUpdateRTprop()
	[]  BBRCheckProbeRTT()
    []
	[X]  BBRSetPacingRate()
	[X]  BBRSetSendQuantum()
	[X]  BBRSetCwnd()

*/
void BbrTcpAgent::recv_newack_helper(Packet *pkt) {
	newack(pkt);

	if(t_rtt_>0){
		// Estimamos BtlBw

		hdr_tcp *tcph = hdr_tcp::access(pkt);
		printf("SEQ: %d\n",tcph->seqno());

		//hdr_cmn* ch = hdr_cmn::access(pkt);
		//int psize = ch->size();

		prior_inflight = inflight();
		inflight = packets_in_flight();

		UpdateBtlBw(delivered_rate);
		CheckCyclePhase();
		CheckFullPipe();
		CheckDrain();
		UpdateRTprop((int)t_rtt_);
		CheckProbeRTT();

		double bdp = BtlBw * RTprop;
		double b_inflight = BytesInFlight();

		if(bbr_mode == BBR_DRAIN){
			pacing_gain = bbr_drain_gain;
			if(b_inflight <= bdp){
				bbr_mode = BBR_PROBE_BW;
				bbrmodetmr.resched(BBR_PROBE_BW_TIME);
			}
		}

		else if(bbr_mode == BBR_STARTUP){
			if (filled_pipe){
				bbr_mode= BBR_DRAIN;
				pacing_gain = bbr_drain_gain;
			}else{
				pacing_gain = bbr_high_gain;
			}
		}

		SetPacingRateWithGain(pacing_gain);
		SetSendQuantum();
		UpdateTargetCwnd(); // CREO

		printf("MinRTT: %.2f RTT: %.2f MaxBtlBw: %.2f BtlBw: %.2f Mode: %d Gain: %.2f Inflight: %.2f\n",RTprop,(double)t_rtt_,BtlBw, delivered_rate, bbr_mode, pacing_gain,inflight);

	}

	/*
	if (app_limited_until > 0){
     app_limited_until = app_limited_until - psize;
	}
	*/

	/* if the connection is done, call finish() */
	if ((highest_ack_ >= curseq_-1) && !closed_) {
		closed_ = 1;
		finish();
	}
}

void BbrTcpAgent::timeout(int tno){
	if (tno == TCP_TIMER_RTX) {
		dupacks_ = 0;
		recover_ = maxseq_;
		last_cwnd_action_ = CWND_ACTION_TIMEOUT;
		reset_rtx_timer(0);
		++nrexmit_;
		send_much(0, TCP_REASON_TIMEOUT,1);
	} else {
		send_much(1, TCP_REASON_TIMEOUT,1);
	}

	if (tno == bbr_mode_CHANGE){
		if(bbr_mode== BBR_PROBE_BW){
			bbr_mode = BBR_PROBE_RTT;
			bbrmodetmr.resched(BBR_PROBE_RTT_TIME);
			return;
		}
		if(bbr_mode== BBR_PROBE_RTT){
			bbr_mode = BBR_PROBE_BW;
			bbrmodetmr.resched(BBR_PROBE_BW_TIME);
			return;
		}
	}
}
double BbrTcpAgent::BytesInFlight(){
	double sol = ndatabytes_ - delivered;
	return (sol < 0) ? 0 : sol;
}

void BbrTcpAgent::output(int seqno, int reason){
	int is_retransmit = (seqno < maxseq_);
	Packet* p = allocpkt();
	hdr_tcp *tcph = hdr_tcp::access(p);
	tcph->seqno() = seqno;
	double now = Scheduler::instance().clock();
	tcph->ts() = now;
	tcph->ts_echo() = ts_peer_;
	tcph->last_rtt() = int(int(t_rtt_)*tcp_tick_*1000);
	tcph->reason() = reason;
	int bytes = hdr_cmn::access(p)->size();

	//int bdp = BtlBw * RTprop;
	//int inflight = BytesInFlight();

	if(seqno==0){
		if(syn_){
			bytes = 0;
			curseq_ +=1;
			hdr_cmn::access(p)->size() = tcpip_base_hdr_size_;
		}
	}else if (useHeaders_ == true){
		hdr_cmn::access(p)->size() += headersize();
	}
	output_helper(p);
	ndatabytes_ += bytes;
	ndatapack_++; // Added this - Debojyoti 12th Oct 2000
	send(p, 0);

	// If the packet is the last
	if (seqno == curseq_ && seqno > maxseq_){
		idle();
	}
	// If the packet hasnt been transmitted
	if (seqno > maxseq_) {
		maxseq_ = seqno;
		if (!rtt_active_) {
			rtt_active_ = 1;
			if (seqno > rtt_seq_) {
				rtt_seq_ = seqno;
				rtt_ts_ = now;
			}
		}
	} else {
		++nrexmitpack_;
    nrexmitbytes_ += bytes;
  }

	if(bbr_mode==BBR_PROBE_BW){
  	pacing_gain =  getProbeBWGain();
	}
	if(BtlBw!=0){
		bbr_next_sent = bytes*1.0 / (pacing_gain *BtlBw * 1.0);
	}else{
		bbr_next_sent = 0.25;
	}

	if(bbr_mode==BBR_PROBE_RTT || bbr_next_sent > 0.25){
		bbr_next_sent = 0.25;
	}

	if (!(rtx_timer_.status() == TIMER_PENDING)){
		set_rtx_timer();
	}

	printf("SENT:%.lf bytes: %d Next: %.2f Gain: %.2f RTprop: %.2f BtlBw: %.2f Mode: %d\n",Now(),bytes,bbr_next_sent,pacing_gain,RTprop,BtlBw,bbr_mode);

}

void BbrTcpAgent::send_one(){
	if(t_seqno_ < curseq_) return;
	output(t_seqno_,0);
	t_seqno_ ++;
}

/*
 * return -1 if the oldest sent pkt has not been timeout (based on
 * fine grained timer).
 */
int BbrTcpAgent::bbr_expire(Packet* pkt){
	hdr_tcp *tcph = hdr_tcp::access(pkt);
	return(tcph->seqno()+1);
	//return(-1);
}

void BbrTcpAgent::recv(Packet *pkt, Handler *)
{
	hdr_tcp *tcph = hdr_tcp::access(pkt);
	hdr_flags *flagh = hdr_flags::access(pkt);
	++nackpack_;
	int valid_ack = 0;
	if (flagh->ecnecho()){
		ecn(tcph->seqno());
	}
	if (tcph->seqno() > last_ack_) {
		recv_newack_helper(pkt);
   }
	else if (tcph->seqno() == last_ack_)  {
		/* check if a timeout should happen */
		++dupacks_;
		int expired=bbr_expire(pkt);
		if (expired>=0 || dupacks_ == numdupacks_) {
			if ((highest_ack_ > recover_) || ( last_cwnd_action_ != CWND_ACTION_TIMEOUT)) {
				last_cwnd_action_ = CWND_ACTION_DUPACK;
				recover_ = maxseq_;
				reset_rtx_timer(1);
				if(expired>=0){
					output(expired, TCP_REASON_DUPACK);
				}
				else {
					output(last_ack_ + 1, TCP_REASON_DUPACK);
				}
      }
		}
	}
	if(tcph->seqno()>= last_ack_){
		valid_ack = 1;
	}
	Packet::free(pkt);

	if (dupacks_ == 0 || dupacks_ > numdupacks_ - 1)
		send_much(0, 0, maxburst_);
}

void BbrTcpAgent::send_much(int force, int reason, int maxburst)
{	cwnd_ = 1;
	wnd_ = 1;
	send_idle_helper();
	int npackets = 0;

	/* Save time when first packet was sent, for newreno  --Allman */
	if (t_seqno_ == 0)
		firstsent_ = Scheduler::instance().clock();

	//if (!setup_maxburst(&force, &maxburst)) return;

	// Si todavía no se han enviado todos los paquetes
	if (t_seqno_ < curseq_) {
		if (overhead_ == 0 || force || qs_approved_ ) {
			output(t_seqno_, reason);
			npackets++;
			if (QOption_){
				process_qoption_after_send () ;
			}
			t_seqno_ ++ ;
			if (qs_approved_ == 1) {
					delsnd_timer_.resched(bbr_next_sent);
        if (!npackets) send_blocked_++;
				return;
			}
		}
		else if (!(delsnd_timer_.status() == TIMER_PENDING)) {
			delsnd_timer_.resched(bbr_next_sent);
			if (!npackets) send_blocked_++;
			return;
		}
		if (maxburst && npackets == maxburst)
			return;
	}
  if (!npackets) send_blocked_++;
  if (t_seqno_ <= highest_ack_ && t_seqno_ < curseq_)
		bbrpactimer.resched(bbr_next_sent);

	/* call helper function */
	send_helper(maxburst);
}

void BbrTcpAgent::EnterStartup(){
       bbr_mode = BBR_STARTUP;
       pacing_gain = bbr_high_gain;
       cwnd_gain = bbr_high_gain;
}
/*
   [?]    init_windowed_max_filter(filter=BBR.BtlBwFilter, value=0, time=0)
   [x]    BBR.rtprop = SRTT ? SRTT : Inf
   [x]    BBR.rtprop_stamp = Now()
   [x]    BBR.probe_rtt_done_stamp = 0

   []    BBR.probe_rtt_round_done = false
   []    BBR.packet_conservation = false
   []    BBR.prior_cwnd = 0
   []    BBR.idle_restart = false

   [?]    BBRInitRoundCounting()
   [x]    BBRInitFullPipe()
   [x]    BBRInitPacingRate()
   [x]    BBREnterStartup()

 */
void BbrTcpAgent::BbrInit(){

	//CONSTANTS
	bbr_high_gain  = 2.885f;
	bbr_drain_gain = 1.f/bbr_high_gain;
	bbr_cwnd_gain  = 2;

	bbr_recoverypacing_rounds = 8;
	ProbeRTTDuration = BBR_PROBE_RTT_TIME*1000; //msec
	MinPipeCwnd = 4;

	bbr_pacing_i = 0;

	// NEW VARIAVBLES
	probe_rtt_done_stamp = 0;
	probe_rtt_round_done = false;
	packet_conservation = false;
	prior_cwnd = 0;
	idle_restart = false;

	// RTprop
	RTprop_stamp = 0;
	RTpropFilterLen = 10;
	RTprop = ((int)t_srtt_) ? ((int)t_srtt_) : -1;

	// BtlWb (round Counting, I think)
	BtlBw_stamp = 10;
	BtlBwFilterLen = 0;
	BtlBw = -1;

	// FullPipe
	filled_pipe = false;
	bandwidth_at_last_round_ = 0;
	rounds_without_bandwidth_gain_ = 0;

	InitPacingRate();
	EnterStartup();

}
