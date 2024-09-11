/* $Id$ */

#include "../config.h"

#include "kn_cgi.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
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
		if((path = kn_find(MANPAGE_DIR, kn_get_query("page"))) != NULL) {
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
	printf("html {\n");
	printf("	background-color: #222222;\n");
	printf("	color: #ffffff;\n");
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
	printf("				Name: <input name=\"page\">\n");
	printf("				<input type=\"submit\">\n");
	printf("			</form>\n");
	printf("		</div>\n");
	printf("		<hr>\n");
	if(no) {
		printf("		Not found.\n");
	} else if(showmain) {
		printf("%s", MAIN_HTML);
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
	printf("		<i>Generated by <a href=\"http://nishi.boats/keine\">Keine</a> %s</i>\n", kn_get_version());
	printf("	</body>\n");
	printf("</html>\n");
	if(path != NULL) free(path);
}
