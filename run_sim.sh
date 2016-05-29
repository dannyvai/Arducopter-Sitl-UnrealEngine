#!/bin/bash

Ardupilot_folder=~/ardupilot/


pkill -9 -f ArduCopter.elf
pkill -9 -f sim_wrapper.py
pkill -9 -f mavproxy.py

screen -wipe

while [[ $# > 0 ]]
do
    key="$1"
    case $key in
        kill)
            shift
            exit
            ;;
    esac   
    shift
done

cd $Ardupilot_folder/ArduCopter/
make sitl
cd -

cp ./ardupilot/Tools/autotest/pysim/sim_wrapper.py $Ardupilot_folder/Tools/autotest/pysim/

screen -S 'arducopter' -dm bash -c "$Ardupilot_folder/ArduCopter/ArduCopter.elf -S -I0 --home -35.363261,149.165230,584,353; sleep 1000"
sleep 1
screen -S 'sim_wrapper' -dm bash -c "python $Ardupilot_folder/Tools/autotest/pysim/sim_wrapper.py --home=-35.363261,149.165230,584,353 --simin=127.0.0.1:5502 --simout=127.0.0.1:5501 --fgout=127.0.0.1:5503; sleep 1000"
sleep 1
screen -S 'mavproxy' -m bash -c "mavproxy.py --master tcp:127.0.0.1:5760; sleep 1000"
sleep 1
