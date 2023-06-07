#! /bin/bash
	for j in 0 1
	do
			echo "move data from /buff${j}/uwb_test/band${j}/pol_a/* to /data/uwb_test/band${j}/pol_a/, please waiting ..."
			mv -ig /buff${j}/uwb_test/band${j}/pol_a/* /data/uwb_test/band${j}/pol_a/
			echo "move data from uwb-gpu2 /buff${j}/uwb_test/band${j}/pol_a/* to /data/uwb_test/band$(expr 2 + $j)/pol_a/, please waiting ..."
			scp -P 5907 uwb-gpu2:/buff${j}/uwb_test/band${j}/pol_a/* /data/uwb_test/band$(expr 2 + $j)/pol_a/
			echo "remove data from uwb-gpu2 /buff${j}/uwb_test/band${j}/pol_a, please waiting ..."
			ssh -t -p 5907 uwb-gpu2 rm /buff${j}/uwb_test/band${j}/pol_a/uwb_input*

	done
