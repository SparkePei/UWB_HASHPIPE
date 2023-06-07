#!/bin/bash
DATADIR=$1
file=`/bin/ls  ${DATADIR}uwb_input*`
#file=`/bin/ls -1 ${DATADIR}/uwb_input* ${DATADIR}/*.FIL 2>/dev/null | head -1`
for i in $file
do 
	echo mv $i $i.vdif
	mv $i $i.vdif
done

#for i in 2 3 4 5
#do
#	#echo hashpipe -p ./snap2_hashpipe -I $i -o BINDHOST="enp134s0f1" -o BINDPORT=$(expr 6000 + $i - 1) -c $(expr 14 + $i) snap2_net_thread -c $(expr 34 + $i) snap2_output_thread \&
#	echo hashpipe -p ./snap2_hashpipe -I $i -o BINDHOST="enp134s0f1" -o BINDPORT=$(expr 6000 + $i - 1) -m $((0x3000 << $i)) snap2_net_thread -c $(expr 34 + $i) snap2_output_thread \&
#done
