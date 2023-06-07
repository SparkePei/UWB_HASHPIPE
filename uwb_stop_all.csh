#! /bin/tcsh

date

foreach i (uwb-gpu1 uwb-gpu2)          # Nanshan
    echo $i"..."
    ssh $i /usr/local/bin/uwb_stop.sh
end

