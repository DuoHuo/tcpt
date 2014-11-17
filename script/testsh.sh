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
if [ `mount | grep tcptfs | wc -l` -eq 0 ]; then
    mount -t tcptfs /sdcard/test/ /sdcard/test/
fi
