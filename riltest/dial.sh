# !/bin/sh
rm /data/dial_result
rldial
while busybox [  ! -e  "/data/dial_result"  ]
do
	sleep 1
	echo "redial"
	rldial
done

ipaddr=$(grep address /data/dial_result)
busybox ifconfig eth_x ${ipaddr##address} netmask 255.255.255.0 up
echo busybox ifconfig eth_x ${ipaddr##address} netmask 255.255.255.0 up

gateway=$(grep gateway /data/dial_result)
busybox route del default > /dev/null 2>&1
busybox route add default gw ${gateway##gateway}
echo "busybox route add default gw ${gateway##gateway}"
