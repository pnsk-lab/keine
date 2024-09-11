/* $Id$ */

#include <stdio.h>

#include "kn_cgi.h"

int main() {
	printf("Content-Type: text/html\n");
	kn_parse_query();
	kn_cgi();
	return 0;
}
