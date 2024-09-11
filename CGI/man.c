/* $Id$ */

#include "../config.h"

#include "kn_man.h"

#include "kn_util.h"

#include <stdbool.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

char* kn_find(const char* root, const char* name) {
	DIR* dir = opendir(root);
	if(dir != NULL) {
		struct dirent* d;
		while((d = readdir(dir)) != NULL) {
			if(strcmp(d->d_name, "..") != 0 && strcmp(d->d_name, ".") != 0) {
				char* path = kn_strcat3(root, "/", d->d_name);
				struct stat s;
				if(stat(path, &s) == 0) {
					if(S_ISDIR(s.st_mode)) {
						char* v;
						if((v = kn_find(path, name)) != NULL) {
							closedir(dir);
							return v;
						}
					} else if(strcmp(d->d_name, name) == 0) {
						closedir(dir);
						return path;
					}
				}
				free(path);
			}
		}
		closedir(dir);
	}
	return NULL;
}

bool kn_has_manpage(const char* str) {
	char* pth = kn_find(MANPAGE_DIR, str);
	if(pth == NULL) return false;
	free(pth);
	return true;
}

char* kn_manpage_process(const char* path) {
	char* b = malloc(1);
	b[0] = 0;
	int pipes[2];
	char cbuf[2];
	cbuf[1] = 0;
	pipe(pipes);
	pid_t pid = fork();
	if(pid == 0) {
		close(pipes[0]);
		dup2(pipes[1], STDOUT_FILENO);
		execlp("man", "man", path, NULL);
		_exit(1);
	} else {
		close(pipes[1]);
		char c;
		bool sta = false;
		char s = 0;
		char old = 0;
		char m = 0;
		while(1) {
			if(read(pipes[0], &c, 1) <= 0) break;
			if(c == 8) {
				sta = true;
			} else {
				if(s != 0) {
					if(sta) {
						sta = false;
						old = s;
					} else {
						if(old == 0) {
							char* tmp;
							if(m == 'B') {
								tmp = b;
								b = kn_strcat(b, "</b>");
								free(tmp);
							} else if(m == 'U') {
								tmp = b;
								b = kn_strcat(b, "</u>");
								free(tmp);
							}
							m = 0;
							cbuf[0] = s;
							tmp = b;
							b = kn_strcat(b, cbuf);
							free(tmp);
						} else {
							if(old == s) {
								cbuf[0] = s;
								char* tmp;
								if(m == 'U') {
									tmp = b;
									b = kn_strcat(b, "</u>");
									free(tmp);
								}
								if(m != 'B') {
									tmp = b;
									b = kn_strcat(b, "<b>");
									free(tmp);
								}
								m = 'B';
								tmp = b;
								b = kn_strcat(b, cbuf);
								free(tmp);
							} else if(old == '_') {
								cbuf[0] = s;
								char* tmp;
								if(m == 'B') {
									tmp = b;
									b = kn_strcat(b, "</b>");
									free(tmp);
								}
								if(m != 'U') {
									tmp = b;
									b = kn_strcat(b, "<u>");
									free(tmp);
								}
								tmp = b;
								b = kn_strcat(b, cbuf);
								free(tmp);
								tmp = b;
								b = kn_strcat(b, "</u>");
								free(tmp);
							}
							old = 0;
						}
					}
				}
				s = c;
			}
		}
		waitpid(pid, 0, 0);
		char* tmp;
		if(m == 'B') {
			tmp = b;
			b = kn_strcat(b, "</b>");
			free(tmp);
		} else if(m == 'U') {
			tmp = b;
			b = kn_strcat(b, "</u>");
			free(tmp);
		}
	}
	return b;
}
