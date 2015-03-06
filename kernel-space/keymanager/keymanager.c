#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#define DEVICE_NAME "key_manager"
#define MAJOR_NUM 138
#define KEY_LEN 20
#define KEY_RING_LEN	128

#define IOC_STOREKEY	0xee
#define IOC_CLEARKEY	0xef

uint8_t keyring[KEY_RING_LEN];
EXPORT_SYMBOL(keyring);

long device_ioctl(
                 struct file *file,
                 unsigned int ioctl_num,
                 unsigned long ioctl_param)
{
	int ret_val;
        switch (ioctl_num) {
        	case IOC_STOREKEY:
			printk("Storing keyring\n");
        		ret_val = copy_from_user(keyring, (char *)ioctl_param, KEY_LEN); //TODO
        		break;
		case IOC_CLEARKEY:
			printk("Clearing keyring\n");
			memset(keyring,0,KEY_RING_LEN);
			break;
        }

        return 0;
}


struct file_operations fops = {
        .unlocked_ioctl = device_ioctl,
};

static int init_keymanager(void)
{
        int ret_val;

        ret_val = register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops);
        if (ret_val < 0) {
                printk("failed with %d\n", ret_val);
                return ret_val;
        }
	printk("keymanager module inserted\n");
        return 0;
}

static void cleanup_keymanager(void)
{
	unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
	printk("keymanager module removed\n");
}

MODULE_LICENSE("Dual BSD/GPL");
module_init(init_keymanager);
module_exit(cleanup_keymanager);
