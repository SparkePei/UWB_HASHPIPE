#! /bin/bash

date

for i in uwb-gpu1 uwb-gpu2          # Nanshan
do
    ssh -p 5907 $i /usr/local/bin/uwb_stop.sh &
done

