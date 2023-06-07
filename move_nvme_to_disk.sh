#! /bin/bash
for i in uwb-gpu1 uwb-gpu2
do
	for j in 0 1
	do
		for k in a b
		do	
			echo "move data from $i /buff${j}/uwb_test/band${j}/pol_${k}/ to /data/uwb_test/band${j}/pol_${k}/, please waiting ..."
			ssh -t -p 5907 $i mv -ig /buff${j}/uwb_test/band${j}/pol_${k}/* /data/uwb_test/band${j}/pol_${k}/
		done
	done
done
