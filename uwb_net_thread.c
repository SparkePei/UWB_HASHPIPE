/*
 * uwb_net_thread.c
 *
 * This allows you to receive pakets from local ethernet, unpack the packets, and then write data into a shared memory buffer. 
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sched.h>
#include <smmintrin.h>
#include <immintrin.h>
#include "hashpipe.h"
#include <hiredis/hiredis.h>
#include "uwb_databuf.h"

#define PKTSOCK_BYTES_PER_FRAME (16384)
#define PKTSOCK_FRAMES_PER_BLOCK (8)
#define PKTSOCK_NBLOCKS (20)
#define PKTSOCK_NFRAMES (PKTSOCK_FRAMES_PER_BLOCK * PKTSOCK_NBLOCKS)

double MJD;
int     band_ID;
bool start_file=0;

static int init(hashpipe_thread_args_t * args)
{
	// define network params
	char bindhost[80];
	int bindport = 60001;
	hashpipe_status_t st = args->st;
	strcpy(bindhost, "0.0.0.0");
	
	hashpipe_status_lock_safe(&st);
	// Get info from status buffer if present (no change if not present)
	hgets(st.buf, "BINDHOST", 80, bindhost);
	hgeti4(st.buf, "BINDPORT", &bindport);
	// Store bind host/port info etc in status buffer
	hputs(st.buf, "BINDHOST", bindhost);
	hputi4(st.buf, "BINDPORT", bindport);
	hputi8(st.buf, "NPACKETS", 0);
	hputi8(st.buf, "PKTSIZE", SIZE_OF_PKT);
	hashpipe_status_unlock_safe(&st);

	// Set up pktsock 
	struct hashpipe_pktsock *p_ps = (struct hashpipe_pktsock *)
	malloc(sizeof(struct hashpipe_pktsock));
	if(!p_ps) {
		perror(__FUNCTION__);
		return -1;
	}
	/* Make frame_size be a divisor of block size so that frames will be
	contiguous in mapped memory.  block_size must also be a multiple of
	page_size.  Easiest way is to oversize the frames to be 16384 bytes, which
	is bigger than we need, but keeps things easy. */
	p_ps->frame_size = PKTSOCK_BYTES_PER_FRAME;
	// total number of frames
	p_ps->nframes = PKTSOCK_NFRAMES;
	// number of blocks
	p_ps->nblocks = PKTSOCK_NBLOCKS;
	int rv = hashpipe_pktsock_open(p_ps, bindhost, PACKET_RX_RING);
	if (rv!=HASHPIPE_OK) {
	hashpipe_error("uwb_net_thread", "Error opening pktsock.");
	pthread_exit(NULL);
	}
	// Store packet socket pointer in args
	args->user_data = p_ps;
	// Success!
	return 0;
}
//typedef struct {
//    uint64_t mcnt;
//    uint64_t pchan;	// First chan in packet
//    uint64_t nchan;	// Number of channels in packet
//    uint64_t sid;	// Source ID (aka band id)
//} packet_header_t;

typedef struct {
	uint32_t    sec_ref;
	uint32_t    data_frame;
	uint64_t	seq;	
	//int	Band_ID;
} packet_header_t;

static inline void get_header(unsigned char *p_frame, packet_header_t * pkt_header)
{
	//uint64_t raw_header;
	//raw_header = *(unsigned long long *)PKT_UDP_DATA(p_frame);
    uint32_t sec_ref;
    uint32_t data_frame;
    //unsigned char band_id;
//    raw_header = le64toh(*(unsigned long long *)p->data);
    //memcpy(&sec_ref,packet+12,4*sizeof(char));
    memcpy(&sec_ref,PKT_UDP_DATA(p_frame),4*sizeof(char));
    memcpy(&data_frame,PKT_UDP_DATA(p_frame)+4,4*sizeof(char));
    //memcpy(&band_id,PKT_UDP_DATA(p_frame)+22,1*sizeof(char));
    //pkt_header->sec_ref       = sec_ref & 0x3fffffff;
    pkt_header->sec_ref       = sec_ref & 0xfffff;
    pkt_header->data_frame       = data_frame & 0x00ffffff;
    pkt_header->seq        = (sec_ref & 0x3fffffff) * 62500 + (data_frame & 0x00ffffff);
    //pkt_header->Band_ID    = (int)band_id;
    //band_ID = (int)band_id;
    //if (TEST){
    #ifdef TEST_MODE
            fprintf(stderr,"**Header**\n");
            fprintf(stderr,"second of reference is :%ld \n ",pkt_header->sec_ref);
            fprintf(stderr,"data frame is :%ld \n ",pkt_header->data_frame);
            fprintf(stderr,"pakect seq is :%ld \n ",pkt_header->seq);
            //fprintf(stderr,"Band ID is:%d\n\n",pkt_header->Band_ID);
    #endif
        //}
}

double UTC2JD(double year, double month, double day){
	double jd;
	double a;
	a = floor((14-month)/12);
	year = year+4800-a;
	month = month+12*a-3;
	jd = day + floor((153*month+2)/5)+365*year+floor(year/4)-floor(year/100)+floor(year/400)-32045;
	return jd;
}

static void *run(hashpipe_thread_args_t * args){
	uwb_input_databuf_t *db  = (uwb_input_databuf_t *)args->obuf;
	hashpipe_status_t st = args->st;
	const char * status_key = args->thread_desc->skey;

	int i, rv,input,n;
	uint32_t mcnt = 0;
	int block_idx = 0;
	unsigned long header; // 64 bit counter     
	// unsigned char data_pkt[SIZE_OF_PKT]; // save received packet
	packet_header_t pkt_header;
	unsigned long SEQ=0;
	unsigned long LAST_SEQ=0;
	unsigned long CHANNEL;
	unsigned long n_pkt_rcv; // number of packets has been received
	unsigned long pkt_loss; // number of packets has been lost
	int first_pkt=1;
	double pkt_loss_rate; // packets lost rate
	unsigned int pktsock_pkts = 0;  // Stats counter from socket packet
	unsigned int pktsock_drops = 0; // Stats counter from socket packet
	double Year, Month, Day;
	double jd;
	time_t timep;
	struct tm *p;
	struct timeval currenttime;
	time(&timep);
	p=gmtime(&timep);
	Year=p->tm_year+1900;
	Month=p->tm_mon+1;
	Day=p->tm_mday;
	jd = UTC2JD(Year, Month, Day); 
	MJD=jd+(double)((p->tm_hour-12)/24.0)
                               +(double)(p->tm_min/1440.0)
                               +(double)(p->tm_sec/86400.0)
                               +(double)(currenttime.tv_usec/86400.0/1000000.0)
								-(double)2400000.5;
	printf("MJD time of packets is %lf\n",MJD);

	uint64_t npackets = 0; //number of received packets
	int bindport = 0;
	//sleep(1);
	
	redisContext *redis_c;
        redisReply *reply;
        const char *hostname = "uwb-gpu1";
        //const char *hostname = "localhost";
        //const char *hostname = "xbd3";
        int redis_port = 6379;

        struct timeval timeout = { 1, 500000 }; // 1.5 seconds
        redis_c = redisConnectWithTimeout(hostname, redis_port, timeout);
        if (redis_c == NULL || redis_c->err) {
                if (redis_c) {
                        printf("Connection error: %s\n", redis_c->errstr);
                        redisFree(redis_c);
                } else {
                        printf("Connection error: can't allocate redis context\n");
                }
                exit(1);
        }

        /* Give all the threads a chance to start before opening network socket */
        sleep(2);
        //sleep(3);
        /* Get receiving flag from redis server */
        printf("waiting for set start_flag to 1 on server to start ...\n");
        do {
                reply = (redisReply *)redisCommand(redis_c,"GET start_flag");
        sleep(0.1);
        } while(strcmp(reply->str,"1")!=0); // if start_flag set to 1 then start data receiving.


        printf("GET value from %s and start_flag is: %s, start data receiving...\n", hostname, reply->str);
        freeReplyObject(reply);
        // wait until the integer time value changes.
        time_t time0=time(&timep);
        while(time(&timep)==time0);


	hashpipe_status_lock_safe(&st);
	// Get info from status buffer if present (no change if not present)
	hgeti4(st.buf, "BINDPORT", &bindport);
	hputs(st.buf, status_key, "running");
	hashpipe_status_unlock_safe(&st);

	// Get pktsock from args
	struct hashpipe_pktsock * p_ps = (struct hashpipe_pktsock*)args->user_data;
	pthread_cleanup_push(free, p_ps);
	pthread_cleanup_push((void (*)(void *))hashpipe_pktsock_close, p_ps);

	// Drop all packets to date
	unsigned char *p_frame;
	while(p_frame=hashpipe_pktsock_recv_frame_nonblock(p_ps)) {
		hashpipe_pktsock_release_frame(p_frame);
	}

	// Main loop
	while (run_threads()){		
		hashpipe_status_lock_safe(&st);
		hputs(st.buf, status_key, "waiting");
		hputi4(st.buf, "NETBKOUT", block_idx);
		hputi4(st.buf,"NETMCNT",mcnt);
		hashpipe_status_unlock_safe(&st);

		// Wait for data
		/* Wait for new block to be free, then clear it
		 * if necessary and fill its header with new values.
		 */
		while ((rv=uwb_input_databuf_wait_free(db, block_idx)) 
			    != HASHPIPE_OK) {
			if (rv==HASHPIPE_TIMEOUT) {
			    hashpipe_status_lock_safe(&st);
			    hputs(st.buf, status_key, "blocked");
			    hashpipe_status_unlock_safe(&st);
			    continue;
			} else {
			    hashpipe_error(__FUNCTION__, "error waiting for free databuf");
			    pthread_exit(NULL);
			    break;
			}
		}

		hashpipe_status_lock_safe(&st);
		hputs(st.buf, status_key, "receiving");
		hashpipe_status_unlock_safe(&st);

		// receiving packets
		for(int i=0;i<N_PKTS_PER_BUF;i++){
			do {
				p_frame = hashpipe_pktsock_recv_udp_frame_nonblock(p_ps, bindport);
			} 
			while (!p_frame && run_threads());
			if(!run_threads()) break;
 			if(npackets == 0){
				get_header(p_frame,&pkt_header);
				SEQ = pkt_header.seq;
				pkt_loss=0;
				LAST_SEQ = (SEQ-1);
			}
			npackets++;

			hashpipe_pktsock_release_frame(p_frame);
			// copy data to input data buffer                             
			//printf("db->block[block_idx]+%d\n",(i%(PAGE_SIZE*N_PKTS_PER_SPEC))*DATA_SIZE_PER_PKT); 
			// Use length from packet (minus UDP header and minus HEADER word and minus CRC word)
			// memcpy(dest_p, payload_p, PKT_UDP_SIZE(p_frame) - 8 - 8 - 8);
			// memcpy(db->block[block_idx].data_block+(i%(PAGE_SIZE*N_PKTS_PER_SPEC))*DATA_SIZE_PER_PKT,(PKT_UDP_DATA(p_frame)+8),DATA_SIZE_PER_PKT*sizeof(unsigned char));
			memcpy(db->block[block_idx].data_in+i*DATA_SIZE_PER_PKT, PKT_UDP_DATA(p_frame), DATA_SIZE_PER_PKT*sizeof(unsigned char));
			//memcpy(db->block[block_idx].data_in+i*DATA_SIZE_PER_PKT, PKT_UDP_DATA(p_frame)+8, DATA_SIZE_PER_PKT*sizeof(unsigned char));
			// memcpy(db->block[block_idx].data_block+i*DATA_SIZE_PER_PKT, PKT_UDP_DATA(p_frame)-8-8-8, DATA_SIZE_PER_PKT*sizeof(unsigned char));
			pthread_testcancel();
		}

		#ifdef TEST_MODE
			printf("number of lost packets is : %lu\n",pkt_loss);
		#endif
		// Handle variable packet size!
		int packet_size = PKT_UDP_SIZE(p_frame) - 40;
		//int packet_size = PKT_UDP_SIZE(p_frame) - 8;
		#ifdef TEST_MODE
			printf("packet size is: %d\n",packet_size);
		#endif
		get_header(p_frame,&pkt_header);
		SEQ = pkt_header.seq;
		//printf("SEQ is : %lu\n",SEQ);
		pkt_loss += SEQ - (LAST_SEQ+N_PKTS_PER_BUF);
		pkt_loss_rate = (double)pkt_loss/(double)npackets*100.0;
		LAST_SEQ = SEQ;

		// Get stats from packet socket
		hashpipe_pktsock_stats(p_ps, &pktsock_pkts, &pktsock_drops);

		hashpipe_status_lock_safe(&st);
		hputi8(st.buf, "NPACKETS", npackets);
		hputi8(st.buf,"PKTLOSS",pkt_loss);
		hputr8(st.buf,"LOSSRATE",pkt_loss_rate);		
		hputu8(st.buf, "NETRECV",  pktsock_pkts);
		hputu8(st.buf, "NETDROPS", pktsock_drops);
		hashpipe_status_unlock_safe(&st);

		// Mark block as full
		if(uwb_input_databuf_set_filled(db, block_idx) != HASHPIPE_OK) {
			hashpipe_error(__FUNCTION__, "error waiting for databuf filled call");
			pthread_exit(NULL);
		}

		db->block[block_idx].header.netmcnt = mcnt;
		block_idx = (block_idx + 1) % db->header.n_block;
		mcnt++;

		/* Will exit if thread has been cancelled */
		pthread_testcancel();
	}
	pthread_cleanup_pop(1); /* Closes push(hashpipe_pktsock_close) */
	pthread_cleanup_pop(1); /* Closes push(free) */
	// Thread success!
	return THREAD_OK;
}

static hashpipe_thread_desc_t uwb_net_thread = {
	name: "uwb_net_thread",
	skey: "NETSTAT",
	init: init,
	run:  run,
	ibuf_desc: {NULL},
	obuf_desc: {uwb_input_databuf_create}
};

static __attribute__((constructor)) void ctor()
{
  register_hashpipe_thread(&uwb_net_thread);
}
