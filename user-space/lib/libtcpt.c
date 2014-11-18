#include "libtcpt.h"
#include <fcntl.h>

/**
 * Hash user's new password and store in file(the shadow)
 */
int reg_pwd(char *pwd)
{
	uint8_t result[16];
	FILE *filp;
    int size;

	md5((uint8_t *)pwd, strlen(pwd), result);
	filp = fopen(MD5_PATH,"wb+");
	if (!filp) {
        LOGI("fopen err %s\n",MD5_PATH);
		return -1;
	}
	size = fwrite(result, 16, 1, filp);
    if (!size) {
        LOGI("fwrite err %s\n",MD5_PATH);
        fclose(filp);
        return -1;
    }
	fclose(filp);
	return 1;
}

/**
 * Compare password with the shadow, return positive if success
 * if success, store key into the kernel keymanager via ioctl
 */
int authen_store_key(char *pwd)
{
	uint8_t source[16];
	uint8_t target[16];
	FILE *filp;
	int dev_filp;
	uint8_t sha1key[20];
	SHA1Context sha;
	int err;

	/* Compare md5 value */
	md5((uint8_t *)pwd, strlen(pwd), target);
	filp = fopen(MD5_PATH,"rb");
	if (!filp) {
		return -1;
	}
	fread(source, 16, 1, filp);
	if (memcmp(target, source, 16)) {
        fclose(filp);
		return -1;
	}
	fclose(filp);
	/* sha1key */
	err = SHA1Reset(&sha);
	if (err) {
		return -1;
	}
	err = SHA1Input(&sha, (const unsigned char *)pwd, strlen(pwd));
	if (err) {
		return -1;
	}
	err = SHA1Result(&sha, sha1key);
	if (err) {
		return -1;
	}
	/* Store key into keymanager */
    	dev_filp = open("/dev/keymanager",0);
    	if (dev_filp < 0) {
    		return -1;
    	}
    	ioctl(dev_filp, 0xee, sha1key);
	close(dev_filp);
	return 1;
}

/* Clear key which stored in keymanager. Use this at the right time */
int clear_key()
{
	int dev_filp;

	dev_filp = open("/dev/keymanager",0);
	if (dev_filp<0) {
		return -1;
	}
	ioctl(dev_filp, 0xef, NULL);
	close(dev_filp);
	return 1;
}

#if 0
/**
 * Mount secure path now and modify the init script so the path
 * will be automatically mounted when power on
 */
int mount_list(char *pathlist)
{
	return 0;
}

/**
 * Make nod for keymanager device file
 */
int mknod_for_keymanager()
{
	return 0;
}
#endif
