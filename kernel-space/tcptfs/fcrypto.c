#include "tcptfs.h"
#include <linux/crypto.h>

int tcptfs_decrypt_page(struct page *page)
{
	struct page *enc_page = NULL;
	char *enc_page_virt;
	char iv[16] = "789054654327876";
	struct scatterlist src_sg, dst_sg;
	struct crypto_blkcipher *tfm;
	struct blkcipher_desc desc = {
		.info = iv,
		.flags = CRYPTO_TFM_REQ_MAY_SLEEP
	};
	int rc = 0;

	enc_page = alloc_page(GFP_USER);
	if (!enc_page) {
		rc = -ENOMEM;
		pr_err("Error allocating page for encrypted page\n");
		goto out_page;
	}
	enc_page_virt = kmap(enc_page);
	
	sg_init_table(&src_sg, 1);
	sg_init_table(&dst_sg, 1);
	sg_set_page(&src_sg, page, PAGE_CACHE_SIZE, 0);
	sg_set_page(&dst_sg, enc_page, PAGE_CACHE_SIZE, 0);

	tfm = crypto_alloc_blkcipher("ecb(aes)", 0, CRYPTO_ALG_ASYNC);
	if (!tfm) {
		printk("failed to alloc tfm\n");
		rc = -EINVAL;
		goto out;
	}
	crypto_blkcipher_set_flags(tfm, CRYPTO_TFM_REQ_WEAK_KEY);
	desc.tfm = tfm;
	rc = crypto_blkcipher_setkey(tfm, keyring, 16);
	if (rc) {
		pr_err("Error setting key\n");
		rc = -EINVAL;
		goto out;
	}
	rc = crypto_blkcipher_decrypt_iv(&desc, &dst_sg, &src_sg, PAGE_CACHE_SIZE);

out:
	crypto_free_blkcipher(tfm);
	
out_page:
	if (enc_page) {
		kunmap(enc_page);
		__free_page(enc_page);
	}
	return rc;
	return 0;
}

int tcptfs_encrypt_page(struct page *page)
{
	struct page *enc_page = NULL;
	char *enc_page_virt;
	char iv[16] = "789054654327876";
	struct scatterlist src_sg, dst_sg;
	struct crypto_blkcipher *tfm;
	struct blkcipher_desc desc = {
		.info = iv,
		.flags = CRYPTO_TFM_REQ_MAY_SLEEP
	};
	int rc = 0;

	enc_page = alloc_page(GFP_USER);
	if (!enc_page) {
		rc = -ENOMEM;
		pr_err("Error allocating page for encrypted page\n");
		goto out_page;
	}
	enc_page_virt = kmap(enc_page);
	
	sg_init_table(&src_sg, 1);
	sg_init_table(&dst_sg, 1);
	sg_set_page(&src_sg, page, PAGE_CACHE_SIZE, 0);
	sg_set_page(&dst_sg, enc_page, PAGE_CACHE_SIZE, 0);

	tfm = crypto_alloc_blkcipher("ecb(aes)", 0, CRYPTO_ALG_ASYNC);
	if (!tfm) {
		printk("failed to alloc tfm\n");
		rc = -EINVAL;
		goto out;
	}
	crypto_blkcipher_set_flags(tfm, CRYPTO_TFM_REQ_WEAK_KEY);
	desc.tfm = tfm;
	rc = crypto_blkcipher_setkey(tfm, keyring, 16);
	if (rc) {
		pr_err("Error setting key\n");
		rc = -EINVAL;
		goto out;
	}
	crypto_blkcipher_encrypt_iv(&desc, &dst_sg, &src_sg, PAGE_CACHE_SIZE);

out:
	crypto_free_blkcipher(tfm);
	
out_page:
	if (enc_page) {
		kunmap(enc_page);
		__free_page(enc_page);
	}
	return rc;
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
