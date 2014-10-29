#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "md5.h"

int main()
{
	uint8_t buf[16];
	int i;

	md5((uint8_t *)"hello", strlen("hello"), buf);
	for (i=0; i<16; i++) {
		printf("%x", buf[i]);
	}
	return 0;
}
