#!/bin/bash

args=""

for node in $@; do
	proc="$(ps aux | grep "./ls_router ${node} " | grep -v "grep" )"
	pid=$(echo $proc | sed -r 's/^([^.]+).*$/\1/; s/^[^0-9]*([0-9]+).*$/\1/')
	#echo $proc
	#echo $pid
	$(kill ${pid})
done

