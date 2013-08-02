#!bin/sh
#written by lgy.

#stop all tcpdump processes
ps -e|grep "tcpdump"|awk '{print $1}'|xargs kill -9
