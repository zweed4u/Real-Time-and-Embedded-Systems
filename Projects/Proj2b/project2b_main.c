/*
 * Demonstrate how to set up a timer that, on expiry,
 * sends us a pulse.  This example sets the first
 * expiry to i.5 seconds and the repetition interval
 * to i.5 seconds.
 */

#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>

#define MY_PULSE_CODE   _PULSE_CODE_MINAVAIL

typedef union {
        struct _pulse   pulse;
        /* your other message structures would go
           here too */
} my_message_t;



int main(void){
	int i=0;
	struct sigevent         event;
	struct itimerspec       itime;
	timer_t                 timer_id;
	int                     chid, rcvid;
	my_message_t            msg;

	chid = ChannelCreate(0);

	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, chid,_NTO_SIDE_CHANNEL, 0);
	event.sigev_priority = getprio(0);
	event.sigev_code = MY_PULSE_CODE;
	timer_create(CLOCK_REALTIME, &event, &timer_id);

	itime.it_value.tv_sec = 1;
	/* 500 million nsecs = .5 secs */
	itime.it_value.tv_nsec = 500000000;
	itime.it_interval.tv_sec = 1;
	/* 500 million nsecs = .5 secs */
	itime.it_interval.tv_nsec = 500000000;
	timer_settime(timer_id, 0, &itime, NULL);

	/*
	 * As of the timer_settime(), we will receive our pulse
	 * in 1.5 seconds (the itime.it_value) and every 1.5
	 * seconds thereafter (the itime.it_interval)
	 */
	for (;;) {
		itime.it_value.tv_sec = i; //increments the time +1 second on each pass for pulse
		/* 500 million nsecs = .5 secs */
		itime.it_value.tv_nsec = 500000000;
		itime.it_interval.tv_sec = i; //increments the time +1 second on each pass for pulse
		/* 500 million nsecs = .5 secs */
		itime.it_interval.tv_nsec = 500000000;
		timer_settime(timer_id, 0, &itime, NULL);
		rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
		if (rcvid == 0) { /* we got a pulse */
        	if (msg.pulse.code == MY_PULSE_CODE) {
            	printf("we got a pulse from our timer\n");
            } /* else other pulses ... */
		} /* else other messages ... */
		i+=1;
	}
	return 0;
}
