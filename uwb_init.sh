#!/bin/bash
# Remove old semaphore
echo removing old semaphore, if any
#sudo hashpipe_clean_shmem
#sudo rm /dev/shm/sem.home_peix_hashpipe_status_*
for i in 0 1
#for i in 1
#for i in 0
do
	#hashpipe -p ./uwb_hashpipe -I $i -o BINDHOST="enp134s0f0" -o BINDPORT=$(expr 60001 + $i) -m $((0x3000 << $i)) -o FILDIR="/buff$i/uwb_test/" uwb_net_thread -c $(expr 34 + $i) uwb_output_thread &
	echo hashpipe -p /home/peix/hashpipe/uwb_hashpipe/uwb_hashpipe -I $i -o BINDHOST="enp64s0f${i}np$i" -o BINDPORT=60000 -m $((0x100000100 << $i)) -o FILDIR="/buff$i/uwb_test/band$i/pol_a/" -c $(expr 8 + $i) uwb_net_thread -c $(expr 32 + $i) uwb_output_thread \&
	#hashpipe -p /home/peix/hashpipe/uwb_hashpipe/uwb_hashpipe -I $i -o BINDHOST="enp64s0f${i}np$i" -o BINDPORT=60000 -o FILDIR="/buff$i/uwb_test/band$i/pol_a/" uwb_net_thread  uwb_output_thread &
	#hashpipe -p /home/peix/hashpipe/uwb_hashpipe/uwb_hashpipe -I $i -o BINDHOST="enp64s0f${i}np$i" -o BINDPORT=60000 -o FILDIR="/buff$i/uwb_test/band$i/pol_a/" -c $(expr 8 + $i),$(expr 10 + $i) uwb_net_thread -c $(expr 32 + $i),20 uwb_output_thread &
	#hashpipe -p /home/peix/hashpipe/uwb_hashpipe/uwb_hashpipe -I $i -o BINDHOST="enp64s0f${i}np$i" -o BINDPORT=60000 -o FILDIR="/buff$i/uwb_test/band$i/pol_a/" -c $(expr 8 + $i) uwb_net_thread -c $(expr 32 + $i) uwb_output_thread &
	hashpipe -p /home/peix/hashpipe/uwb_hashpipe/uwb_hashpipe -I $i -o BINDHOST="enp64s0f${i}np$i" -o BINDPORT=60000 -m $((0x100000100 << $i)) -o FILDIR="/buff$i/uwb_test/band$i/pol_a/" -c $(expr 8 + $i) uwb_net_thread -c $(expr 32 + $i) uwb_output_thread &

	#echo hashpipe -p /home/peix/hashpipe/uwb_hashpipe/uwb_hashpipe -I $(expr 2 + $i) -o BINDHOST="enp64s0f${i}np$i" -o BINDPORT=60001 -m $((0x400000400 << $i)) -o FILDIR="/buff$i/uwb_test/band$i/pol_b/" -c $(expr 10 + $i) uwb_net_thread -c $(expr 34 + $i) uwb_output_thread \&
	#hashpipe -p /home/peix/hashpipe/uwb_hashpipe/uwb_hashpipe -I $(expr 2 + $i) -o BINDHOST="enp64s0f${i}np$i" -o BINDPORT=60001 -o FILDIR="/buff$i/uwb_test/band$i/pol_b/" uwb_net_thread uwb_output_thread &
	#hashpipe -p /home/peix/hashpipe/uwb_hashpipe/uwb_hashpipe -I $(expr 2 + $i) -o BINDHOST="enp64s0f${i}np$i" -o BINDPORT=60001 -o FILDIR="/buff$i/uwb_test/band$i/pol_b/" -c $(expr 10 + $i) uwb_net_thread -c $(expr 34 + $i) uwb_output_thread &
	#hashpipe -p /home/peix/hashpipe/uwb_hashpipe/uwb_hashpipe -I $(expr 2 + $i) -o BINDHOST="enp64s0f${i}np$i" -o BINDPORT=60001 -m $((0x400000400 << $i)) -o FILDIR="/buff$i/uwb_test/band$i/pol_b/" -c $(expr 10 + $i) uwb_net_thread -c $(expr 34 + $i) uwb_output_thread &
	#hashpipe -p /home/peix/hashpipe/uwb_hashpipe/uwb_hashpipe -I $i -o BINDHOST="enp64s0f${i}np$i" -o BINDPORT=60001 -o FILDIR="/buff$i/uwb_test/band$i/pol_a/"  uwb_net_thread  uwb_output_thread &
	#hashpipe -p ./uwb_hashpipe -I $i -o BINDHOST="enp6f$i" -o BINDPORT=60001 -o FILDIR="/buff$i/uwb_test/" -c $(expr 14 + $i) uwb_net_thread -c $(expr 34 + $i) uwb_output_thread &
        #hashpipe -p ./uwb_hashpipe -I $i -o BINDHOST="enp134s0f1" -o BINDPORT=$(expr 60000 + $i - 1) -c $(expr 11 + $i) uwb_net_thread -c $(expr 34 + $i) uwb_output_thread &
done
	#hashpipe -p ./uwb_hashpipe -I $i -o BINDHOST="enp134s0f1" -o BINDPORT=60001 -m 0xc000 uwb_net_thread -c 36 uwb_output_thread & 
#hashpipe -p ./uwb_hashpipe -I 0 -o BINDHOST="10.0.0.147" -o BINDPORT=60001 -m 0x4000 uwb_net_thread -c 14 uwb_output_thread 
#hashpipe -p ./uwb_hashpipe -I 0 -o BINDHOST="10.0.0.147" -o BINDPORT=60001 -c 12 uwb_net_thread -c 14 uwb_output_thread 
#hashpipe -p ./uwb_hashpipe -I 3 -o BINDHOST="enp134s0f1" -o BINDPORT=60002 -m 0x30000 uwb_net_thread -c 37 uwb_output_thread &
#hashpipe -p ./uwb_hashpipe -I 4 -o BINDHOST="enp134s0f1" -o BINDPORT=60002 -m 0x30000 uwb_net_thread -c 37 uwb_output_thread &
#hashpipe -p ./uwb_hashpipe -I 5 -o BINDHOST="enp134s0f1" -o BINDPORT=60002 -m 0x30000 uwb_net_thread -c 37 uwb_output_thread &

