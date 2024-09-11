/* $Id$ */

#include "../config.h"

#include "kn_cgi.h"

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <stdio.h>

#include "kn_version.h"
#include "kn_util.h"
#include "kn_man.h"

struct q_entry {
	char* key;
	char* value;
};

bool no = false;
bool showmain = false;
struct q_entry** entries = NULL;
char* path;

char* kn_get_query(const char* key) {
	if(entries == NULL) return NULL;
	int i;
	for(i = 0; entries[i] != NULL; i++) {
		if(strcmp(entries[i]->key, key) == 0) return entries[i]->value;
	}
	return NULL;
}

char* kn_null(const char* a) { return a == NULL ? "" : (char*)a; }

void manpage_scan(const char* root) {
	struct dirent** nl;
	int n = scandir(root, &nl, NULL, alphasort);
	if(n < 0) return;
	int i;
	for(i = 0; i < n; i++) {
		if(strcmp(nl[i]->d_name, ".") != 0 && strcmp(nl[i]->d_name, "..") != 0) {
			char* path = kn_strcat3(root, "/", nl[i]->d_name);
			struct stat s;
			if(stat(path, &s) == 0) {
				if(S_ISDIR(s.st_mode)) {
					manpage_scan(path);
				} else {
					char* name = kn_strdup(nl[i]->d_name);
					char* desc = kn_strdup("<span class=\"reverse\"> &lt;No description detected&gt; </span>");

					int incr = 0;
					FILE* f = fopen(path, "r");
					char* b = malloc(s.st_size + 1);
					b[s.st_size] = 0;
					fread(b, s.st_size, 1, f);
					fclose(f);

					int j;
					for(j = 0;; j++) {
						if(b[j] == '\n' || b[j] == 0) {
							char* line = malloc(j - incr + 1);
							line[j - incr] = 0;
							memcpy(line, b + incr, j - incr);

							int k;
							for(k = 0; line[k] != 0 && k < 4; k++) {
								if(line[k] == ' ') {
									line[k] = 0;
									if(strcasecmp(line, ".Nd") == 0) {
										free(desc);
										desc = kn_strdup(line + k + 1);
										int l;
										for(l = 0; desc[l] != 0; l++) {
											if(desc[l] == '\\') {
												l++;
												if(desc[l] == '"') {
													l--;
													desc[l] = 0;
													break;
												}
											}
										}
									}
									break;
								}
							}

							free(line);
							incr = j + 1;
							if(b[j] == 0) break;
						}
					}

					free(b);

					if(strlen(desc) > 70) {
						desc[70] = 0;
						desc[69] = '.';
						desc[68] = '.';
						desc[67] = '.';
					}

					printf("<tr>\n");
					printf("	<td><a href=\"?page=%s\">%s</a></td>\n", name, name);
					printf("	<td><code>%s</code></td>\n", desc);
					printf("</tr>\n");

					free(name);
					free(desc);
				}
			}
			free(path);
		}
		free(nl[i]);
	}
	free(nl);
}

void list_manpages(void) {
#ifdef MANPAGE_DIRS
	int i;
	const char* dirs[] = MANPAGE_DIRS;
	for(i = 0; i < sizeof(dirs) / sizeof(*dirs); i++) {
		manpage_scan(dirs[i]);
	}
#else
	manpage_scan(MANPAGE_DIR);
#endif
}

void kn_parse_query(void) {
	char* query = getenv("QUERY_STRING");
	if(query != NULL) {
		entries = malloc(sizeof(*entries));
		entries[0] = NULL;
		int i;
		int incr = 0;
		for(i = 0;; i++) {
			if(query[i] == 0 || query[i] == '&') {
				char* a = malloc(i - incr + 1);
				memcpy(a, query + incr, i - incr);
				a[i - incr] = 0;

				char* key = a;
				char* value = "";

				int j;
				for(j = 0; key[j] != 0; j++) {
					if(key[j] == '=') {
						key[j] = 0;
						value = key + j + 1;
						break;
					}
				}

				struct q_entry* e = malloc(sizeof(*e));
				e->key = kn_strdup(key);
				e->value = kn_strdup(value);

				struct q_entry** old = entries;
				for(j = 0; old[j] != NULL; j++)
					;
				entries = malloc(sizeof(*entries) * (j + 2));
				for(j = 0; old[j] != NULL; j++) entries[j] = old[j];
				entries[j] = e;
				entries[j + 1] = NULL;
				free(old);

				free(a);

				incr = i + 1;
				if(query[i] == 0) break;
			}
		}
	}
	if(kn_get_query("page") == NULL) {
		printf("Status: 200 OK\n\n");
		showmain = true;
	} else {
		bool cond = false;
#ifdef MANPAGE_DIRS
		int i;
		const char* dirs[] = MANPAGE_DIRS;
		for(i = 0; i < sizeof(dirs) / sizeof(*dirs); i++) {
			cond = (path = kn_find(dirs[i], kn_get_query("page"))) != NULL;
			if(cond) break;
		}
#else
		cond = (path = kn_find(MANPAGE_DIR, kn_get_query("page"))) != NULL;
#endif
		if(cond) {
			printf("Status: 200 OK\n\n");
		} else {
			printf("Status: 404 Not Found\n\n");
			no = true;
		}
	}
}

void kn_cgi(void) {
	printf("<html>\n");
	printf("	<head>\n");
	printf("		<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n");
	printf("		<title>Keine - ");
	if(no) {
		printf("Not found");
	} else if(showmain) {
		printf("Main");
	} else {
		printf("%s", kn_get_query("page"));
	}
	printf("</title>\n");
	printf("		<style>\n");
	;
	printf("html {\n");
	printf("	background-color: #222222;\n");
	printf("	color: #ffffff;\n");
	printf("}\n");
	printf("html, input, code {\n");
	printf("	font-size: 15px;\n");
	printf("}\n");
	printf(".reverse {\n");
	printf("	color: #222222;\n");
	printf("	background-color: #ffffff;\n");
	printf("}\n");
	printf("body {\n");
	printf("	width: 900px;\n");
	printf("	margin: 0 auto;\n");
	printf("}\n");
	printf("a:link {\n");
	printf("	color: #88f;\n");
	printf("}\n");
	printf("a:visited {\n");
	printf("	color: #44b;\n");
	printf("}\n");
	printf("		</style>\n");
	printf("	</head>\n");
	printf("	<body>\n");
	printf("		<div style=\"text-align: center;\">\n");
	printf("			<form action=\"%s%s\">\n", getenv("SCRIPT_NAME"), kn_null(getenv("PATH_INFO")));
	printf("				<a href=\"%s%s\">Main</a> | ", getenv("SCRIPT_NAME"), kn_null(getenv("PATH_INFO")));
	printf("				Name: <input name=\"page\"%s%s%s>\n", kn_get_query("page") == NULL ? "" : " value=\"", kn_get_query("page") == NULL ? "" : kn_get_query("page"), kn_get_query("page") == NULL ? "" : "\"");
	printf("				<input type=\"submit\">\n");
	printf("			</form>\n");
	printf("		</div>\n");
	printf("		<hr>\n");
	if(no) {
		printf("		Not found.\n");
	} else if(showmain) {
#ifdef MAIN_HTML
		printf("%s", MAIN_HTML);
#else
		printf("<h1>Index</h1>\n");
		printf("<table border=\"0\" style=\"width: 900px;\">\n");
		printf("	<tr>\n");
		printf("		<th style=\"width: 50px;\">Name</th>\n");
		printf("		<th>Description</th>\n");
		printf("	</tr>\n");
		list_manpages();
		printf("</table>\n");
#endif
	} else {
		printf("<pre>");
		char* c = kn_manpage_process(path);
		if(c != NULL) {
			printf("%s", c);
			free(c);
		}
		printf("</pre>\n");
	}
	printf("		<hr>\n");
#ifdef FOOTER_HTML
	printf("%s\n", FOOTER_HTML);
	printf("		<hr>\n");
#endif
	printf("		<i>Generated by <a href=\"http://nishi.boats/keine\">Keine</a> %s</i>\n", kn_get_version());
	printf("	</body>\n");
	printf("</html>\n");
	if(path != NULL) free(path);
}
