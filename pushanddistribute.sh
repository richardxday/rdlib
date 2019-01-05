#!/bin/sh
HOSTNAME="`hostname`"
CODEDIR="`pwd`"
make -j4 && make install && git commit -a && git push
for HOST in abacus pivr richard-thinkpad ; do
	if [ "$HOSTNAME" != "$HOST" ] ; then
		echo "Connecting to $HOST..."
		ssh -t $HOST "cd \"$CODEDIR\" ; git fetch && git rebase && make -j4 && make install"
	fi
done
