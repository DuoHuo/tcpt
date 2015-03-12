#!/system/bin/sh

if [ `lsmod | grep keymanager | wc -l` -eq 0 ]; then
    insmod /sdcard/tcpt/keymanager.ko
fi
if [ `lsmod | grep tcptfs | wc -l` -eq 0 ]; then
    insmod /sdcard/tcpt/tcptfs.ko
fi
if [ `ls /dev | grep keymanager | wc -l` -eq 0 ]; then
    /system/xbin/mknod /dev/keymanager c 138 0
    chmod 777 /dev/keymanager
fi
DIR_COUNT=`wc -l /sdcard/tcpt/mount-list | awk -F " " '{print $1}'`
if [ `mount | grep tcptfs | wc -l` -lt ${DIR_COUNT} ]; then
	while read line
	do
		mount -t tcptfs ${line} ${line}
   	done < '/sdcard/tcpt/mount-list';
fi
