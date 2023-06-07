/*
 * 
 * uwb_output_thread.c
 * 
 */
//#include <stdlib.h>
//#include <cstdio>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "hashpipe.h"
#include "uwb_databuf.h"
#include <sys/time.h>
extern int band_ID;
//extern double net_MJD;
static void *run(hashpipe_thread_args_t * args)
{
	printf("Saved file size is: %f Mbytes.\n ",float(N_BYTES_PER_FILE)/1024/1024);
	// Local aliases to shorten access to args fields
	// Our input buffer happens to be a uwb_ouput_databuf
	uwb_output_databuf_t *db = (uwb_output_databuf_t *)args->ibuf;
	hashpipe_status_t st = args->st;
	const char * status_key = args->thread_desc->skey;
	int rv;
	int N_files=0;
	int block_idx = 0;
	uint64_t N_Bytes_save = 0;
	//uint64_t N_Bytes_file = N_BYTES_PER_FILE;
	int f_full_flag = 1;
	FILE * uwb_file;
	char f_fil[250];

	char fil_dir[128];

	hashpipe_status_lock_safe(&st);
	hgets(st.buf, "FILDIR", 80, fil_dir);
 	hputs(st.buf, "FILDIR",fil_dir);
	hashpipe_status_unlock_safe(&st);

	//sleep(1);
	/* Main loop */
	while (run_threads()) {
		hashpipe_status_lock_safe(&st);
		hputi4(st.buf, "OUTBLKIN", block_idx);
		hputi8(st.buf, "DATSAVMB",(N_Bytes_save/1024/1024));
		hputi4(st.buf, "NFILESAV",N_files);
		hputi4(st.buf, "BANDID",band_ID);
		hputs(st.buf, status_key, "waiting");
		hashpipe_status_unlock_safe(&st);

		// Waiting for data to write on disk
		while ((rv=uwb_output_databuf_wait_filled(db, block_idx))!= HASHPIPE_OK) {
		if (rv==HASHPIPE_TIMEOUT) {
			hashpipe_status_lock_safe(&st);
			hputs(st.buf, status_key, "blocked");
			hputi4(st.buf, "OUTBLKIN", block_idx);
			hashpipe_status_unlock_safe(&st);
			continue;
			} else {
				hashpipe_error(__FUNCTION__, "error waiting for filled databuf");
				pthread_exit(NULL);
				break;
			}
		}
		
		hashpipe_status_lock_safe(&st);
		hputs(st.buf, status_key, "processing");
		hputi4(st.buf, "OUTBLKIN", block_idx);
		hashpipe_status_unlock_safe(&st);
		if (f_full_flag ==1){
			struct tm  *now;
			time_t rawtime;
			printf("\n\nopen new file...\n\n");
			//char fil_dir[] = "/buffer/uwb_test/";
			char t_stamp[50];
	        	time(&rawtime);
			now = localtime(&rawtime);
		        strftime(t_stamp,sizeof(t_stamp), "%Y-%m-%d_%H-%M-%S.vdif.working",now);
	                sprintf(f_fil,"%s%s%s%s" ,fil_dir,"uwb_input","_",t_stamp);
	                //sprintf(f_fil,"%s%s%d%s%s" ,fil_dir,"uwb_input",band_ID,"_",t_stamp);
			//WriteHeader(f_fil_P1,net_MJD);
	
			//printf("write header done!\n");
			f_full_flag = 0;
	
			//if (N_files<=1){
			uwb_file=fopen(f_fil,"w");
		        printf("starting write data to %s\n",f_fil);
                	//fwrite(db->block[block_idx].data_out,1,BUF_SIZE,uwb_file);
			//}
			//else{break;}
			//fwrite(f_fil,1,sizeof(f_fil),uwb_file);
			N_files += 1;
		}
	
                fwrite(db->block[block_idx].data_out,BUF_SIZE,1,uwb_file);
		N_Bytes_save += BUF_SIZE;		
	
		//if (TEST){
                #ifdef TEST_MODE
			printf("**Save Information**\n");
			printf("input_ID:%d \n",band_ID);
			printf("Buffsize: %lu",BUF_SIZE);
			printf("flib_flag:%d\n",f_full_flag);
			printf("Data save:%f\n",float(N_Bytes_save)/1024/1024);
			printf("Total file size:%f\n",float(N_BYTES_PER_FILE)/1024/1024);
			printf("Devide:%lu\n\n",N_Bytes_save % N_BYTES_PER_FILE);
                #endif
			//}

		if (N_Bytes_save >= N_BYTES_PER_FILE){
			f_full_flag = 1;
			N_Bytes_save = 0;
			char f_fil_final[250]={""};
			strncpy(f_fil_final,f_fil,strlen(f_fil)-8);
			fclose(uwb_file);
			rename(f_fil,f_fil_final);
			}

		uwb_output_databuf_set_free(db,block_idx);
		block_idx = (block_idx + 1) % db->header.n_block;

		//Will exit if thread has been cancelled
		pthread_testcancel();

	}
	return THREAD_OK;
}

static hashpipe_thread_desc_t uwb_output_thread = {
	name: "uwb_output_thread",
	skey: "OUTSTAT",
	init: NULL, 
	run:  run,
	ibuf_desc: {uwb_output_databuf_create},
	obuf_desc: {NULL}
};

static __attribute__((constructor)) void ctor()
{
	register_hashpipe_thread(&uwb_output_thread);
}

