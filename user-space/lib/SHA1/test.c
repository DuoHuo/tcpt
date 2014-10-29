

/*
* sha1test.c
*
* Description:
* This file will exercise the SHA-1 code performing the three
* tests documented in FIPS PUB 180-1 plus one which calls
* SHA1Input with an exact multiple of 512 bits, plus a few
* error test checks.
*
* Portability Issues:
* None.
*
*/

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "SHA1.h"

int main()
{
	SHA1Context sha;
	int i,  err;
	uint8_t Message_Digest[20];
	/*
	* Perform SHA-1 tests
	*/
	err = SHA1Reset(&sha);
	if (err) {
		printf("error");
	}
	err = SHA1Input(&sha, (const unsigned char *)"hello", strlen("hello"));
	if (err) {
		printf("error");
	}
	err = SHA1Result(&sha, Message_Digest);
	if (err) {
		printf("error");
	} else {
		for (i=0; i<20; i++) {
			printf("%02X", Message_Digest[i]);
		}
		printf("\n");
	}
	return 0;
}

