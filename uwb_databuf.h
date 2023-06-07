#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "hashpipe.h"
#include "hashpipe_databuf.h"


//#define CACHE_ALIGNMENT         128
#define CACHE_ALIGNMENT         64
//#define N_INPUT_BLOCKS          20 
//#define N_OUTPUT_BLOCKS         20
#define N_INPUT_BLOCKS          5 
#define N_OUTPUT_BLOCKS         5
//#define TEST_MODE		0

#define N_PKTS_PER_BUF		204800			// Number of packets in a buffer
//#define N_PKTS_PER_BUF		20480			// Number of packets in a buffer
#define SIZE_OF_PKT		8256			// Size of single packet
#define N_BYTES_JUNK		32			// Number bytes of junk before header
#define N_BYTES_HEADER		32			// Number Bytes of header
#define DATA_SIZE_PER_PKT	(SIZE_OF_PKT-N_BYTES_JUNK)// Data size per packet

#define N_INPUTS		4			// Number of input signals
#define BUF_SIZE		(unsigned long)(N_PKTS_PER_BUF*DATA_SIZE_PER_PKT) 				//Buffer size with polarations
#define N_BUF_PER_FILE		10
//#define N_BUF_PER_FILE		20
#define N_BYTES_PER_FILE	(unsigned long)(N_BUF_PER_FILE*BUF_SIZE) 			// number of bytes per file saved in disk. 

/**************************** parameters for filterbank header ***********************/
//#define SAMP_TIME		64e-6			// sec, when acc_len=8
#define CLOCK			1024			// MHz
#define START_FREQ		1000			// MHz
#define FFT_CHANS		4096			// MHz, number of FFT channels in ROACH2
#define FREQ_RES		(CLOCK/2.0/FFT_CHANS)	// MHz
#define F_OFF   		(-1*FREQ_RES*N_POST_CHANS_COMB) // MHz
#define F_CH1   		(START_FREQ+CLOCK/2.0-FREQ_RES/2.0)	// 
#define ACC_LEN			32			// accumulation length defined in ROACH2
#define SAMP_TIME		(FFT_CHANS*2.0*ACC_LEN/CLOCK*1.0e-6)			// sec, when acc_len=32

#define FIL_LEN			60			// sec



// Used to pad after hashpipe_databuf_t to maintain cache alignment
typedef uint8_t hashpipe_databuf_cache_alignment[
  CACHE_ALIGNMENT - (sizeof(hashpipe_databuf_t)%CACHE_ALIGNMENT)
];

/* INPUT BUFFER STRUCTURES*/
typedef struct uwb_input_block_header {
   uint64_t	netmcnt;        // Counter for ring buffer
   		
} uwb_input_block_header_t;

typedef uint8_t uwb_input_header_cache_alignment[
   CACHE_ALIGNMENT - (sizeof(uwb_input_block_header_t)%CACHE_ALIGNMENT)
];

typedef struct uwb_input_block {

   uwb_input_block_header_t header;
   uwb_input_header_cache_alignment padding; // Maintain cache alignment
   uint8_t  data_in[BUF_SIZE]; //Input buffer for all channels

} uwb_input_block_t;

typedef struct uwb_input_databuf {
   hashpipe_databuf_t header;
   hashpipe_databuf_cache_alignment padding; // Maintain cache alignment
   uwb_input_block_t block[N_INPUT_BLOCKS];
} uwb_input_databuf_t;


/*
  * OUTPUT BUFFER STRUCTURES
  */
typedef struct uwb_output_block_header {

   uint64_t	mcnt;        // Counter for ring buffer
} uwb_output_block_header_t;

typedef uint8_t uwb_output_header_cache_alignment[
   CACHE_ALIGNMENT - (sizeof(uwb_output_block_header_t)%CACHE_ALIGNMENT)
];

typedef struct uwb_output_block {

   uwb_output_block_header_t header;
   uwb_output_header_cache_alignment padding; // Maintain cache alignment
   uint8_t  data_out[BUF_SIZE]; //Input buffer for all channels

} uwb_output_block_t;

typedef struct uwb_output_databuf {
   hashpipe_databuf_t header;
   hashpipe_databuf_cache_alignment padding; // Maintain cache alignment
   uwb_output_block_t block[N_OUTPUT_BLOCKS];
} uwb_output_databuf_t;

/*
 * INPUT BUFFER FUNCTIONS
 */
hashpipe_databuf_t *uwb_input_databuf_create(int instance_id, int databuf_id);

static inline uwb_input_databuf_t *uwb_input_databuf_attach(int instance_id, int databuf_id)
{
    return (uwb_input_databuf_t *)hashpipe_databuf_attach(instance_id, databuf_id);
}

static inline int uwb_input_databuf_detach(uwb_input_databuf_t *d)
{
    return hashpipe_databuf_detach((hashpipe_databuf_t *)d);
}

static inline void uwb_input_databuf_clear(uwb_input_databuf_t *d)
{
    hashpipe_databuf_clear((hashpipe_databuf_t *)d);
}

static inline int uwb_input_databuf_block_status(uwb_input_databuf_t *d, int block_id)
{
    return hashpipe_databuf_block_status((hashpipe_databuf_t *)d, block_id);
}

static inline int uwb_input_databuf_total_status(uwb_input_databuf_t *d)
{
    return hashpipe_databuf_total_status((hashpipe_databuf_t *)d);
}

static inline int uwb_input_databuf_wait_free(uwb_input_databuf_t *d, int block_id)
{
    return hashpipe_databuf_wait_free((hashpipe_databuf_t *)d, block_id);
}

static inline int uwb_input_databuf_busywait_free(uwb_input_databuf_t *d, int block_id)
{
    return hashpipe_databuf_busywait_free((hashpipe_databuf_t *)d, block_id);
}

static inline int uwb_input_databuf_wait_filled(uwb_input_databuf_t *d, int block_id)
{
    return hashpipe_databuf_wait_filled((hashpipe_databuf_t *)d, block_id);
}

static inline int uwb_input_databuf_busywait_filled(uwb_input_databuf_t *d, int block_id)
{
    return hashpipe_databuf_busywait_filled((hashpipe_databuf_t *)d, block_id);
}

static inline int uwb_input_databuf_set_free(uwb_input_databuf_t *d, int block_id)
{
    return hashpipe_databuf_set_free((hashpipe_databuf_t *)d, block_id);
}

static inline int uwb_input_databuf_set_filled(uwb_input_databuf_t *d, int block_id)
{
    return hashpipe_databuf_set_filled((hashpipe_databuf_t *)d, block_id);
}

/*
 * OUTPUT BUFFER FUNCTIONS
 */

hashpipe_databuf_t *uwb_output_databuf_create(int instance_id, int databuf_id);

static inline void uwb_output_databuf_clear(uwb_output_databuf_t *d)
{
    hashpipe_databuf_clear((hashpipe_databuf_t *)d);
}

static inline uwb_output_databuf_t *uwb_output_databuf_attach(int instance_id, int databuf_id)
{
    return (uwb_output_databuf_t *)hashpipe_databuf_attach(instance_id, databuf_id);
}

static inline int uwb_output_databuf_detach(uwb_output_databuf_t *d)
{
    return hashpipe_databuf_detach((hashpipe_databuf_t *)d);
}

static inline int uwb_output_databuf_block_status(uwb_output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_block_status((hashpipe_databuf_t *)d, block_id);
}

static inline int uwb_output_databuf_total_status(uwb_output_databuf_t *d)
{
    return hashpipe_databuf_total_status((hashpipe_databuf_t *)d);
}

static inline int uwb_output_databuf_wait_free(uwb_output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_wait_free((hashpipe_databuf_t *)d, block_id);
}

static inline int uwb_output_databuf_busywait_free(uwb_output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_busywait_free((hashpipe_databuf_t *)d, block_id);
}
static inline int uwb_output_databuf_wait_filled(uwb_output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_wait_filled((hashpipe_databuf_t *)d, block_id);
}

static inline int uwb_output_databuf_busywait_filled(uwb_output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_busywait_filled((hashpipe_databuf_t *)d, block_id);
}

static inline int uwb_output_databuf_set_free(uwb_output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_set_free((hashpipe_databuf_t *)d, block_id);
}

static inline int uwb_output_databuf_set_filled(uwb_output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_set_filled((hashpipe_databuf_t *)d, block_id);
}


