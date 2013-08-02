#!/bin/sh

if [ $# != 1 ]
then
        echo "delete iptables rule:argement error"
        exit 1
fi

for table in nat ;do
  iptables-save -t $table | grep -w $1 | sed 's/-A/-D/' | while read rule;do iptables -t $table $rule;done
done

ret=`iptables-save -t nat | grep -w $1 | wc -l`
exit $ret
