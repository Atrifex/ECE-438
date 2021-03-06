#!/bin/bash

timestamp() {
     date +"%T"
}

sudo tc qdisc del dev eth1 root 2>/dev/null
sudo tc qdisc add dev eth1 root handle 1:0 netem delay ${3}ms loss ${4}%
# sudo tc qdisc add dev eth1 root handle 1:0 netem delay ${2}ms
sudo tc qdisc add dev eth1 parent 1:1 handle 10: tbf rate 100Mbit burst 40mb latency 25ms

for i in $(seq 1 $1)
do
    timestamp
    echo "Testing iteration ${i}"
    ./reliable_sender 192.168.0.2 4950 sourcefile $2
    timestamp
    echo ""
done
