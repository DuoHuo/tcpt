#include "libtcpt.h"

int main()
{
	char *str = "hello";
	int ret;

	ret = reg_pwd(str);
	if (ret==1) {	
		if (authenticate("hallo")==1) {
			printf("OK\n");
		} else {
			printf("BAD\n");
		}
	}
}
