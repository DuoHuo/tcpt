#include "tcptfs.h"

int tcptfs_decrypt_page(struct page *page)
{
	return 0;
}

int tcptfs_encrypt_page(struct page *page)
{
	return 0;
}

int encrypt(char *buf, size_t buflen, char *key, size_t keylen)
{
	int i;
    
    keylen = 4;
	for (i=0; i<buflen; i++) {
		buf[i] = buf[i] ^ key[i%keylen];
	}
	return 0;
}

int decrypt(char *buf, size_t buflen, char *key, size_t keylen)
{
	int i;

    keylen = 4;
	for (i=0; i<buflen; i++) {
		buf[i] = buf[i] ^ key[i%keylen];
	}
	return 0;
}

#if 0
int encrypt(char *buf, size_t buflen, char *key, size_t keylen)
{
    struct scatterlist sg;
    struct crypto_blkcipher *tfm;
    struct blkcipher_desc desc;
    unsigned char *encrypted;
    struct page *en_page;
	// int iv_len;
	// char iv[128];
	
    tfm = crypto_alloc_blkcipher("ecb(aes)", 0, CRYPTO_ALG_ASYNC);
    if (!tfm) {
    	printk("failed to alloc tfm\n");
    	return -1;
    }
	crypto_blkcipher_setkey(tfm,key,keylen);
    desc.tfm = tfm;
    desc.flags = 0;
	// iv_len = crypto_blkcipher_ivsize(tfm);
	// if (iv_len) {
	//     memset(&iv, 0xff, iv_len);
	//     crypto_blkcipher_set_iv(tfm, iv, iv_len);
	// }
	/* Fill up */
    sg_init_one(&sg, buf, buflen);
 	crypto_blkcipher_encrypt(&desc, &sg, &sg, buflen);
	en_page = sg_page(&sg);
    encrypted = kmap(en_page) + sg.offset;
	hexdump(encrypted,sg.length);
 	return 0;
}


int decrypt(char *buf, size_t buflen, char *key, size_t keylen)
{
    struct scatterlist sg;
    struct crypto_blkcipher *tfm;
    struct blkcipher_desc desc;
    unsigned char *decrypted;
    struct page *de_page;
	
    tfm = crypto_alloc_blkcipher("ecb(aes)", 0, CRYPTO_ALG_ASYNC);
    if (!tfm) {
    	printk("failed to alloc tfm\n");
    	return -1;
    }
	crypto_blkcipher_setkey(tfm,key,keylen);
    desc.tfm = tfm;
    desc.flags = 0;
    sg_init_one(&sg, buf, buflen);
 	crypto_blkcipher_decrypt(&desc, &sg, &sg, buflen);
	de_page = sg_page(&sg);
    decrypted = kmap(de_page) + sg.offset;
	hexdump(decrypted,sg.length);
 	return 0;
}
#endif
