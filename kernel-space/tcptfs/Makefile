TCPTFS_VERSION="0.1"

EXTRA_CFLAGS += -DTCPTFS_VERSION=\"$(TCPTFS_VERSION)\"

obj-$(CONFIG_TCPT_FS) += tcptfs.o

tcptfs-y := dentry.o file.o inode.o main.o super.o lookup.o mmap.o fcrypto.o

clean:
	rm *.o
	rm *.ko
