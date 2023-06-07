#! /bin/bash

# export PATH=/usr/local/cuda/bin:$PATH
# export LD_LIBRARY_PATH=/home/jeffc/local/lib:/usr/local/lib:/usr/local/cuda/lib64:/opt/intel/lib/intel64:$LD_LIBRARY_PATH
#export LD_LIBRARY_PATH=/usr/local/lib:/usr/local/cuda/lib64:/opt/intel/lib/intel64:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/local/lib:/usr/local/cuda/lib64:$LD_LIBRARY_PATH
pkill -f "hashpipe -p uwb_hashpipe" ; /home/peix/hashpipe/uwb_hashpipe/uwb_init.sh
#pkill -f "hashpipe -p uwb_hashpipe" ; /usr/local/bin/uwb_init.sh
hashpipe_check_status -k RUNALWYS -I 0 -s 1
hashpipe_check_status -k RUNALWYS -I 1 -s 1
hashpipe_check_status -k IDLE     -I 0 -s 0
hashpipe_check_status -k IDLE     -I 1 -s 0
hashpipe_check_status -k WEBCNTRL -I 0 -s 1
hashpipe_check_status -k WEBCNTRL -I 1 -s 1
