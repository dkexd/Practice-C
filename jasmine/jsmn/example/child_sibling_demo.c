#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "../jsmn.h"

/*
 * An example of reading JSON from stdin and printing its content to stdout.
 * The dump routine uses the new child/sibling feature, so dont forget to define:
 *     JSMN_FIRST_CHILD_NEXT_SIBLING
 */
void depth_first_dump(const char *js, const jsmntok_t *t, int self, unsigned int level) {
    //print self info
    for (int i = 0; i < level; i++) {
        printf(" ");
    }
    if (t[self].type == JSMN_ARRAY) {
		printf("ARRAY\n");
	} else if (t[self].type == JSMN_OBJECT) {
		printf("OBJECT\n");
	} else {
	    printf("%.*s\n", t[self].end - t[self].start, js+t[self].start);
	}


	if ( (self = t[self].first_child) != -1 ) {
		level += 2;
		depth_first_dump(js, t, self, level);
		while ( (self = t[self].next_sibling) != -1 ) {
			depth_first_dump(js, t, self, level);
		}
	}
}

int main() {
	int r;
	int eof_expected = 0;
	char *js = NULL;
	size_t jslen = 0;
	char buf[BUFSIZ];

	jsmn_parser p;
	jsmntok_t *tok;
	size_t tokcount = 2;

	/* Prepare parser */
	jsmn_init(&p);

	/* Allocate some tokens as a start */
	tok = malloc(sizeof(*tok) * tokcount);
	if (tok == NULL) {
		fprintf(stderr, "malloc(): errno=%d\n", errno);
		return 3;
	}

	for (;;) {
		/* Read another chunk */
		r = fread(buf, 1, sizeof(buf), stdin);
		if (r < 0) {
			fprintf(stderr, "fread(): %d, errno=%d\n", r, errno);
			return 1;
		}
		if (r == 0) {
			if (eof_expected != 0) {
				return 0;
			} else {
				fprintf(stderr, "fread(): unexpected EOF\n");
				return 2;
			}
		}

		js = realloc(js, jslen + r + 1);
		if (js == NULL) {
			fprintf(stderr, "realloc(): errno=%d\n", errno);
			return 3;
		}
		strncpy(js + jslen, buf, r);
		jslen = jslen + r;

again:
		r = jsmn_parse(&p, js, jslen, tok, tokcount);
		if (r < 0) {
			if (r == JSMN_ERROR_NOMEM) {
				tokcount = tokcount * 2;
				tok = realloc(tok, sizeof(*tok) * tokcount);
				if (tok == NULL) {
					fprintf(stderr, "realloc(): errno=%d\n", errno);
					return 3;
				}
				goto again;
			}
		} else {
			//dump(js, tok, p.toknext, 0);
			depth_first_dump(js, tok, 0, 0);
			eof_expected = 1;
		}
	}

	return 0;
}
