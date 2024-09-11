/* $Id$ */

#ifndef __KN_MAN_H__
#define __KN_MAN_H__

#include <stdbool.h>

char* kn_find(const char* root, const char* name);
char* kn_manpage_process(const char* path);
bool kn_has_manpage(const char* str);

#endif
