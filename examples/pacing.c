
+	bind("count_", &count_);	//weixl test
+	bind("next_sent_", &next_sent_); // weixl test
+	bind("cwnd_limited_", &cwnd_limited_);	//weixl test
+	bind("send_blocked_", &send_block_);	//weixl test
+	bind("next_timeout_", &next_timeout_);
+	bind("last_cwnd_action_", &last_cwnd_action_);

+ delay_bind_init_one("pace_packet_");    //weixl
+	delay_bind_init_one("pace_loss_filter_");//weixl
+	delay_bind_init_one("inflation_ratio_");//weixl
+	delay_bind_init_one("pure_rate_control_");//weixl
+	delay_bind_init_one("ack_counting_");	//weixl
+	delay_bind_init_one("count_");		//weixl test
+	delay_bind_init_one("next_sent_"); //weixl test
+	delay_bind_init_one("cwnd_limited_"); //weixl test
+	delay_bind_init_one("send_blocked_");
+	delay_bind_init_one("next_timeout_");
+	delay_bind_init_one("last_cwnd_action_");
+	delay_bind_init_one("random_pace_");
 #endif /* TCP_DELAY_BIND_ALL */


+	if (delay_bind(varName, localName, "pace_packet_", &pace_packet_, tracer)) return TCL_OK;  //weixl
+	if (delay_bind(varName, localName, "pace_loss_filter_", &pace_loss_filter_, tracer)) return TCL_OK;
+	if (delay_bind(varName, localName, "inflation_ratio_", &inflation_ratio_, tracer)) return TCL_OK; 	//weixl
+	if (delay_bind(varName, localName, "pure_rate_control_", &pure_rate_control_, tracer)) return TCL_OK;	//weixl
+	if (delay_bind(varName, localName, "ack_counting_", &ack_counting_, tracer)) return TCL_OK; //weixl
+	if (delay_bind(varName, localName, "count_", &count_, tracer)) return TCL_OK; //weixl test
+        if (delay_bind(varName, localName, "next_sent_", &next_sent_, tracer)) return TCL_OK; //weixl test
+	if (delay_bind(varName, localName, "cwnd_limited_",&cwnd_limited_, tracer)) return TCL_OK; //weixl test
+	if (delay_bind(varName, localName, "send_blocked_",&send_blocked_, tracer)) return TCL_OK;
+	if (delay_bind(varName, localName, "next_timeout_",&next_timeout_, tracer)) return TCL_OK;
+	if (delay_bind(varName, localName, "last_cwnd_action_", &last_cwnd_action_, tracer)) return TCL_OK;
+        if (delay_bind(varName, localName, "random_pace_", &random_pace_, tracer)) return TCL_OK;  //weixl


+			 int(*((TracedInt*) v))*tcp_tick_/4.0);
+        else if ((v == &next_sent_) || (v == &next_timeout_))	//weixl
+                snprintf(wrk, TCP_WRK_SIZE,
+                         "%-8.5f %-2d %-2d %-2d %-2d %s %-6.3f\n",
+                         curtime, addr(), port(), daddr(), dport(),
+                         v->name(), double(*((TracedDouble*) v)));



+	if (pace_loss_filter_ <= 0)
+		loss_filter_time_ = 0;
+	else
+		loss_filter_time_ = -1000.0;
+


+	//weixl: for pacing
+	if (pace_rtt_< -1) {
+		pace_rtt_ = tao;
+		pace_min_rtt_ = tao;
+	} else {
+		if (pace_min_rtt_ > tao) pace_min_rtt_=tao;
+		double weight=cwnd_;
+		weight = weight/3.0;
+		if (weight < 4) weight =4;
+		pace_rtt_ = pace_rtt_ * (weight -1) + tao;
+		pace_rtt_ = pace_rtt_ / weight;
+	};
+	//end
+


+//        if (bugfix_ts_ && ((seqno - highest_ack_) > tss_size_* 0.9)) {
+	if (bugfix_ts_ && packet_in_flight() > tss_size_ * 0.9) {


+
+	if (pace_packet_) {
+		last_sendtime_ = Scheduler::instance().clock();
+		last_overhead_ = this_overhead_;
+		this_overhead_ = 0;
+	}
+


+		if (pure_rate_control_)
+			return (int)wnd_;
+		else
+			return ((cwnd_+inflation_) < wnd_ ? (int)(cwnd_+inflation_) : (int)wnd_);
         }
 }
-
 double TcpAgent::windowd()
 {
-	return (cwnd_ < wnd_ ? (double)cwnd_ : (double)wnd_);
+	if (pure_rate_control_)
+		return (double) wnd_;
+	else
+		return ((cwnd_+inflation_) < wnd_ ? (double)(cwnd_+inflation_) : (double)wnd_);
 }

+bool TcpAgent::setup_maxburst(int* force, int* maxburst)
+//by weixl for pacing: called by send_much before the sending loop to set up a max burst
+//if this function return false, the send_much process should exit (for the burst timer is still pending)
+{
+//	printf("setup_maxburst\n");
+	if (pure_rate_control_) (*maxburst) = 1;
+        if ((pace_packet_)&&(pace_rtt_>-1)) {
+                (*force) = 1;
+                (*maxburst) = 1;
+		if (burstsnd_timer_.status() == TIMER_PENDING) {
+			//non-pacing timer driven
+			if (inflation_) inflation_--;
+			double now = Scheduler::instance().clock();
+			double gap = pace_gap();
+			if ((now-last_sendtime_) < gap) {
+				if ((gap + last_sendtime_) < next_sent_) {
+					next_sent_ = gap + last_sendtime_;
+					burstsnd_timer_.resched(next_sent_ - now);
+				}
+				return false;
+			}
+		} else if (inflation_ratio_) {
+			// burstsnd_timer_ expires. qualified for inflation.
+			if (!is_congestion()) {
+				inflation_=0;
+			} else {
+				cwnd_ += inflation_;
+				bool iscong = is_congestion();
+				cwnd_ -= inflation_;
+				if (iscong) {
+					if ((inflation_+1)*inflation_ratio_ < cwnd_)
+						inflation_++;
+				}
+			}
+		}
+        }
+	return true;
+//	printf("finish setup max burst\n");
+}
+
+double TcpAgent::pace_gap()
+//by weixl for pacing: give the pace gap
+{
+//		printf("->pace_gap\n");
+                double rtt = pace_rtt_;
+                if (recover_ >= highest_ack_)
+                        rtt = pace_min_rtt_;
+                double delay;
+                switch (pace_packet_){
+                        case 2:
+                        if (cwnd_<ssthresh_)
+                                if ((cwnd_+cwnd_) < ssthresh_)
+                                        delay = rtt / (cwnd_ + cwnd_);
+                                else
+                                        delay = rtt / ssthresh_;
+                        else
+                                if (wnd_option_ == 8)
+                                        delay = rtt / (cwnd_ + increase_param());
+                                else
+                                        delay = rtt / (cwnd_+1);
+                        break;
+
+                        case 1: delay = rtt / cwnd_;
+                        break;
+                        default:
+                                printf("Error: pace_packet_=%d\n", pace_packet_);
+                }
+                if (delay<=0) {
+                        double cwnd = cwnd_;
+                        double ss = ssthresh_;
+                         printf("%d: schedule a burst timer with %lf %lf %lf %lf delay.\n", (int) this, delay, pace_rtt_, cwnd, ss);
+                }
+//                printf("<-pace_gap\n");
+
+		if (overhead_) {
+			if (random_pace_)
+				this_overhead_ = Random::uniform(delay*4/5) - delay*2/5 + Random::uniform(overhead_);
+			else
+				this_overhead_ = Random::uniform(overhead_);
+			if ( (delay -last_overhead_ + this_overhead_) <= 0.00000001) this_overhead_ = last_overhead_ - delay + 0.00000001;
+                        //printf("%lf: gap=%lf, overhead=%lf, target=%lf\n", Scheduler::instance().clock(), delay, this_overhead_,  delay + this_overhead_ - last_overhead_);
+
+			return delay + this_overhead_ - last_overhead_;
+		} else
+			return delay;
+}
+void TcpAgent::setup_pace_timer()
+//by weixl for pacing: called by send_much after the sending loop to set up a new
+//pacing timer
+{
+        //by weixl for pacing
+//	printf("setup pacing timer\n");
+        if ((pace_packet_) && (pace_rtt_>-1) /*&& (t_seqno_ <= highest_ack_ + win && t_seqno_ < curseq_)*/) {
+	      double expect_delay = pace_gap();
+              burstsnd_timer_.resched(expect_delay);
+	      next_sent_ = expect_delay + Scheduler::instance().clock();
+        }
+//	printf("finish setup timer\n");
+}
+bool TcpAgent::is_congestion()
+{
+	return packet_in_flight() >= cwnd_;
+}
+int TcpAgent::packet_in_flight() {
+	return (t_seqno_ > highest_ack_)? t_seqno_-highest_ack_ : 0;
+}
+double TcpAgent::packet_in_flight_d() {
+	return (double) packet_in_flight();
+}
+int TcpAgent::congestion_window() {
+	return cwnd_;
+}


@@ -920,10 +1109,18 @@
 	if (t_seqno_ == 0)
 		firstsent_ = Scheduler::instance().clock();

+	if (pace_packet_ == 0)	//by weixl
 	if (burstsnd_timer_.status() == TIMER_PENDING)
 		return;
-	while (t_seqno_ <= highest_ack_ + win && t_seqno_ < curseq_) {
-		if (overhead_ == 0 || force || qs_approved_) {
+
+        //by weixl for pacing
+        if (pace_packet_) {
+		if (!setup_maxburst(&force, &maxburst)) return;
+        }
+        //end for pacing
+	//if (pure_rate_control_) printf("%d %d %d\n", maxburst, win, packet_in_flight());
+	while (packet_in_flight()<=win && t_seqno_ < curseq_) {
+		if (overhead_ == 0 || force || qs_approved_ ) {
 			output(t_seqno_, reason);
 			npackets++;
 			if (QOption_)
@@ -937,6 +1134,8 @@
 				} else {
 					delsnd_timer_.resched(delay);
 				}
+                                if (is_congestion()) cwnd_limited_++; //weixl test
+				if (!npackets) send_blocked_++;
 				return;
 			}
 		} else if (!(delsnd_timer_.status() == TIMER_PENDING)) {
@@ -944,12 +1143,20 @@
 			 * Set a delayed send timeout.
 			 */
 			delsnd_timer_.resched(Random::uniform(overhead_));
+                        if (is_congestion()) cwnd_limited_++; //weixl test
+		        if (!npackets) send_blocked_++;	//weixl test
 			return;
 		}
 		win = window();
 		if (maxburst && npackets == maxburst)
 			break;
 	}
+        if (is_congestion()) cwnd_limited_++; //weixl test
+	if (!npackets) send_blocked_++;	//weixl test
+        //by weixl for pacing
+        if ((pace_packet_) && ((t_seqno_ <= highest_ack_ + win && t_seqno_ < curseq_)))
+		setup_pace_timer();
+
 	/* call helper function */
 	send_helper(maxburst);
 }
@@ -977,6 +1184,7 @@
 void TcpAgent::set_rtx_timer()
 {
 	rtx_timer_.resched(rtt_timeout());
+	next_timeout_ = rtt_timeout() + Scheduler::instance().clock();
 }

 
@@ -1117,6 +1325,83 @@
 void TcpAgent::opencwnd()
 {
 	double increment;
+	if (ack_counting_==1) {
+		//OK we are in pacing. We should enjoy the clear cut of two level structure
+		if (!pure_rate_control_) {
+			int current_wnd_ = packet_in_flight();
+			count_ = nackpack_ - old_nackpack_;
+			if (( count_ < old_cwnd_) && (count_ < current_wnd_)) {
+				return;		//ok, still to finish this RTT.
+			}
+
+			//one RTT is finished by ack counting
+			count_ = 0;
+			old_nackpack_ = nackpack_;
+			old_cwnd_ = current_wnd_;
+		} else {
+		        double now = Scheduler::instance().clock();
+			if ((now - last_opencwnd_) < pace_min_rtt_ ) return;
+			last_opencwnd_ = now;
+		}
+
+		if (cwnd_ < ssthresh_) {
+			cwnd_ = cwnd_ * 2;
+			if (cwnd_ > ssthresh_ ) cwnd_ = ssthresh_;
+			goto byebye;	//ok, slow start
+		}
+
+		switch (wnd_option_) {
+		case 0:
+			cwnd_++;
+			goto byebye;
+		case 1:
+			cwnd_+= increase_num_;
+			goto byebye;
+
+			//for the next few: i need to fix them yet.
+                case 8:
+			/* high-speed TCP */
+			double increment;
+		        if (cwnd_ <= low_window_) {
+				increment = 1;
+       			} else if (cwnd_ >= hstcp_.cwnd_last_ &&
+       			       cwnd_ < hstcp_.cwnd_last_ + cwnd_range_) {
+			// cwnd_range_ can be set to 0 to be disabled,
+			//  or can be set from 1 to 100
+			               increment = hstcp_.increase_last_;
+		       } else {
+                		double p = exp(hstcp_.p1 + log(cwnd_) * hstcp_.p2);
+		                double decrease = decrease_param();
+                		increment = cwnd_ * cwnd_ * p /(1/decrease - 0.5);
+                		hstcp_.cwnd_last_ = cwnd_;
+		                hstcp_.increase_last_ = increment;
+			}
+			if ((last_cwnd_action_ == 0 ||
+				last_cwnd_action_ == CWND_ACTION_TIMEOUT)
+				&& max_ssthresh_ > 0) {
+				if ((max_ssthresh_/2) > increment)
+					increment = max_ssthresh_/2;
+                        }
+                        cwnd_ += increment;
+                        goto byebye;
+                default:
+#ifdef notdef
+                        /*XXX*/
+                        error("illegal window option %d", wnd_option_);
+#endif
+                        abort();
+		}
+	}
+
+	if (ack_counting_ != 0) {
+#ifdef notdef
+                        /*XXX*/
+                        error("ack-counting %d", ack_counting_);
+#endif
+                        abort();
+
+	}
+
 	if (cwnd_ < ssthresh_) {
 		/* slow-start (exponential) */
 		cwnd_ += 1;
@@ -1220,6 +1505,7 @@
 			}
 			cwnd_ += increment;
                         break;
+
 		default:
 #ifdef notdef
 			/*XXX*/
@@ -1228,6 +1514,8 @@
 			abort();
 		}
 	}
+
+byebye:
 	// if maxcwnd_ is set (nonzero), make it the cwnd limit
 	if (maxcwnd_ && (int(cwnd_) > maxcwnd_))
 		cwnd_ = maxcwnd_;
@@ -1241,6 +1529,21 @@
 	double decrease;  /* added for highspeed - sylvia */
 	double win, halfwin, decreasewin;
 	int slowstart = 0;
+
+	double old_cwnd;
+
+	if (pace_loss_filter_>0) {
+		double now = Scheduler::instance().clock();
+		if  ((now-loss_filter_time_)<(pace_loss_filter_ * t_rtt_ * tcp_tick_)) {
+			return;
+		}
+		else
+			loss_filter_time_ = now;
+	}
+        if (pace_loss_filter_<0) {
+		old_cwnd = cwnd_;
+        }
+
 	++ncwndcuts_;
 	if (!(how & TCP_IDLE) && !(how & NO_OUTSTANDING_DATA)){
 		++ncwndcuts1_;
@@ -1249,40 +1552,50 @@
 	if (cwnd_ < ssthresh_)
 		slowstart = 1;
         if (precision_reduce_) {
-		halfwin = windowd() / 2;
+		//halfwin = windowd() / 2;
+		halfwin = packet_in_flight_d()/2;
                 if (wnd_option_ == 6) {
                         /* binomial controls */
-                        decreasewin = windowd() - (1.0-decrease_num_)*pow(windowd(),l_parameter_);
+                        //decreasewin = windowd() - (1.0-decrease_num_)*pow(windowd(),l_parameter_);
+			decreasewin = packet_in_flight_d() - (1.0-decrease_num_)*pow(packet_in_flight_d(),l_parameter_);
                 } else if (wnd_option_ == 8 && (cwnd_ > low_window_)) {
                         /* experimental highspeed TCP */
 			decrease = decrease_param();
 			//if (decrease < 0.1)
 			//	decrease = 0.1;
 			decrease_num_ = decrease;
-                        decreasewin = windowd() - (decrease * windowd());
+                        //decreasewin = windowd() - (decrease * windowd());
+			decreasewin = packet_in_flight_d() - (decrease * packet_in_flight_d());
                 } else {
-	 		decreasewin = decrease_num_ * windowd();
+	 		//decreasewin = decrease_num_ * windowd();
+			decreasewin = decrease_num_ * packet_in_flight_d();
 		}
-		win = windowd();
+		//win = windowd();
+		win = packet_in_flight_d();
 	} else  {
 		int temp;
-		temp = (int)(window() / 2);
+		//temp = (int)(window() / 2);
+		temp = (int)(packet_in_flight()/2);
 		halfwin = (double) temp;
                 if (wnd_option_ == 6) {
                         /* binomial controls */
-                        temp = (int)(window() - (1.0-decrease_num_)*pow(window(),l_parameter_));
+                        //temp = (int)(window() - (1.0-decrease_num_)*pow(window(),l_parameter_));
+			temp = (int)(packet_in_flight() - (1.0-decrease_num_)*pow(packet_in_flight(),l_parameter_));
                 } else if ((wnd_option_ == 8) && (cwnd_ > low_window_)) {
                         /* experimental highspeed TCP */
 			decrease = decrease_param();
 			//if (decrease < 0.1)
                         //       decrease = 0.1;
 			decrease_num_ = decrease;
-                        temp = (int)(windowd() - (decrease * windowd()));
+                        //temp = (int)(windowd() - (decrease * windowd()));
+			temp = (int)(packet_in_flight_d() - (decrease * packet_in_flight_d()));
                 } else {
- 			temp = (int)(decrease_num_ * window());
+ 			//temp = (int)(decrease_num_ * window());
+			temp = (int)(decrease_num_ * packet_in_flight());
 		}
 		decreasewin = (double) temp;
-		win = (double) window();
+		//win = (double) window();
+    	win = (double) packet_in_flight();
 	}
 	if (how & CLOSE_SSTHRESH_HALF)
 		// For the first decrease, decrease by half
@@ -1321,7 +1634,7 @@
 	else if (how & CLOSE_CWND_HALF_WAY) {
 		// cwnd_ = win - (win - W_used)/2 ;
 		cwnd_ = W_used + decrease_num_ * (win - W_used);
-                if (cwnd_ < 1)
+                if (cwnd_ < 1)
                         cwnd_ = 1;
 	}
 	if (ssthresh_ < 2)
@@ -1340,8 +1653,10 @@
 		trace_event("SLOW_START");


+        if (pace_loss_filter_<0) {
+                cwnd_ = old_cwnd +  (cwnd_ - old_cwnd) / (-pace_loss_filter_);
+        }

-
 }

 /*
@@ -1728,7 +2043,9 @@
 		/*
 		 * slow start, but without retransmissions
 		 */
-		cwnd_ = 1; break;
+		cwnd_ = 1;
+		break;
+
 	}

 	/*
@@ -1847,6 +2164,11 @@
 	 	*/
 		send_much(1, TCP_REASON_TIMEOUT, maxburst_);
 	}
+
+        /* added by weixl for per packet pacing */
+        if (tno == TCP_TIMER_BURSTSND) {
+                send_much(0, TCP_TIMER_BURSTSND, maxburst_);
+        }
 }

 void TcpAgent::timeout(int tno)
@@ -1859,10 +2181,12 @@

 		frto_ = 0;
 		// Set pipe_prev as per Eifel Response
-		pipe_prev_ = (window() > ssthresh_) ?
-			window() : (int)ssthresh_;
+//		pipe_prev_ = (window() > ssthresh_) ?
+//			window() : (int)ssthresh_;
+		pipe_prev_ = (packet_in_flight() > ssthresh_) ? packet_in_flight() : (int) ssthresh_;

-	        if (cwnd_ < 1) cwnd_ = 1;
+	        if (cwnd_ < 1)
+			cwnd_ = 1;
 		if (qs_approved_ == 1) qs_approved_ = 0;
 		if (highest_ack_ == maxseq_ && !slow_start_restart_) {
 			/*
@@ -2010,7 +2334,8 @@
 	switch (how) {
 	case 0:
 		/* timeouts */
-		ssthresh_ = int( window() / 2 );
+		//ssthresh_ = int( window() / 2 );
+		ssthresh_ = int (packet_in_flight() / 2 );
 		if (ssthresh_ < 2)
 			ssthresh_ = 2;
 		cwnd_ = int(wnd_restart_);
@@ -2019,7 +2344,8 @@
 	case 1:
 		/* Reno dup acks, or after a recent congestion indication. */
 		// cwnd_ = window()/2;
-		cwnd_ = decrease_num_ * window();
+		//cwnd_ = decrease_num_ * window();
+		cwnd_ = decrease_num_ * packet_in_flight();
 		ssthresh_ = int(cwnd_);
 		if (ssthresh_ < 2)
 			ssthresh_ = 2;
@@ -2037,7 +2363,8 @@
 		break;
 	case 4:
 		/* Tahoe dup acks */
-		ssthresh_ = int( window() / 2 );
+		//ssthresh_ = int( window() / 2 );
+		ssthresh_ = int( packet_in_flight() / 2);
 		if (ssthresh_ < 2)
 			ssthresh_ = 2;
 		cwnd_ = 1;
@@ -2071,7 +2398,8 @@
 			W_used = 0 ;
 		}
 		T_last = tcp_now ;
-		if (t_seqno_ == highest_ack_+ window()) {
+//		if (t_seqno_ == highest_ack_+ window()) {		???? I am not sure here
+		if (t_seqno_ == highest_ack_+ packet_in_flight()) {
 			T_prev = tcp_now ;
 			W_used = 0 ;
 		}
@@ -2133,7 +2461,8 @@
 		T_start  = tcp_now ;
 		F_full = 0;
 	}
-	if (t_seqno_ == highest_ack_ + window()) {
+//	if (t_seqno_ == highest_ack_ + window()) {		???? not sure here too
+	if (t_seqno_ == highest_ack_ + packet_in_flight()) {
 		W_used = 0 ;
 		F_full = 1 ;
 		RTT_prev = RTT_count ;
```
