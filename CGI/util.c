/* $Id$ */

#include "kn_util.h"

#include <string.h>
#include <stdlib.h>

char* kn_strcat(const char* a, const char* b){
	char* str = malloc(strlen(a) + strlen(b) + 1);
	memcpy(str, a, strlen(a));
	memcpy(str + strlen(a), b, strlen(b));
	str[strlen(a) + strlen(b)] = 0;
	return str;
}

char* kn_strdup(const char* a){
	return kn_strcat(a, "");
}
