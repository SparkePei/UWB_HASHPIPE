#! /bin/bash

date

for i in uwb-gpu1 uwb-gpu2       # Nanshan
do
    #echo $i
    #echo "    killing old uwb (if any)..."
    #ssh -p 5907 $i /usr/local/bin/uwb_stop.sh 
    #echo "    starting redis gateway..."
    #ssh $i  hashpipe_redis_gateway.rb
    #echo "    starting uwb..."
    ssh -p 5907 $i /usr/local/bin/uwb_restart.sh &
    #echo " "
done

