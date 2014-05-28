# !/bin/sh
rm /data/dial_result
rldial
while busybox [  ! -e  "/data/dial_result"  ]
do
	sleep 1
	echo "redial"
	rldial
done

ipaddr=$(busybox grep address /data/dial_result)
busybox ifconfig eth_x ${ipaddr##address} netmask 255.255.255.0 up
echo busybox ifconfig eth_x ${ipaddr##address} netmask 255.255.255.0 up

gateway=$(busybox grep gateway /data/dial_result)
busybox route del default > /dev/null 2>&1
busybox route add default gw ${gateway##gateway}
echo "busybox route add default gw ${gateway##gateway}"
busybox arp -i eth_x -s ${gateway##gateway} 58:02:03:04:05:06
echo "busybox arp -i eth_x -s ${gateway##gateway} 58:02:03:04:05:06"

pdns=$(busybox grep pdns /data/dial_result)
sdns=$(busybox grep sdns /data/dial_result)
echo "dnrd -s ${pdns##pdns}" -s ${sdns##sdns}" 
dnrd -s ${pdns##pdns}" -s ${sdns##sdns}  &

# iptables -F
# iptables -t nat -F
# iptables -t filter -F
# iptables -X
# iptables -P FORWARD ACCEPT
# echo "iptables -t nat -A POSTROUTING -s 192.168.1.0/24 -o eth_x -j SNAT --to ${ipaddr##address}"
# iptables -t nat -A POSTROUTING -s 192.168.1.0/24 -o eth_x -j SNAT --to ${ipaddr##address}


echo 1 > /proc/sys/net/ipv4/ip_forward

iptables -F 
iptables -X 
iptables -Z 
iptables -F -t nat 
iptables -X -t nat 
iptables -Z -t nat 
iptables -P INPUT ACCEPT
iptables -P OUTPUT ACCEPT 
iptables -P FORWARD ACCEPT 
iptables -t nat -P PREROUTING ACCEPT 
iptables -t nat -P POSTROUTING ACCEPT 
iptables -t nat -P OUTPUT ACCEPT 
iptables -t nat -A POSTROUTING -s 192.168.1.0/24 -j MASQUERADE
echo "iptables -t nat -A POSTROUTING -s 192.168.1.0/24 -o eth_x -j SNAT --to ${ipaddr##address}"
iptables -t nat -A POSTROUTING -s 192.168.1.0/24 -o eth_x -j SNAT --to ${ipaddr##address}

dnrd -s 

# busybox route add -net 192.168.1.0  netmask 255.255.255.0  br0  
# busybox route add -net 10.174.154.0 netmask 255.255.255.0  eth_x  
# busybox route add -net  0.0.0.0  netmask  0.0.0.0 gw 10.174.154.201 eth_x  
