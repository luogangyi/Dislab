#!/bin/sh


vmcnt=`xm list | grep -w $1 | grep -v grep | wc -l`

if [ $vmcnt == 1 ]
then
        exit 0
fi

exit 1