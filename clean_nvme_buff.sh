#! /bin/bash
for i in uwb-gpu1 uwb-gpu2
do
	for j in 0 1
	do
		for k in a b
		do	
			echo "remove data from $i /buff${j}/uwb_test/band${j}/pol_${k}, please waiting ..."
			ssh -p 5907 $i rm /buff${j}/uwb_test/band${j}/pol_${k}/uwb_input*
		done
	done
done
