/*
    2xml -- convert line-oriented file format to XML.
    Copyright (C) 2000  Daniel Egnor

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <ctype.h>

#include <libxml/HTMLparser.h>

int do_html;
int in_tag = 0;
int do_format = 0;
int indent_level = 0;
int text_was_printed = 0; /* New: Flag to track if text was just output */

struct node 
{
	char *name;
	struct node *child;
};

struct node *root = NULL;

static void print_indent()
{
	if (!do_format) return;
	int i;
	for (i = 0; i < indent_level; i++) {
		fputs("  ", stdout);
	}
}

static void finish_tag()
{
	if (in_tag) {
		putchar('>');
		in_tag = 0;
	}
}

static void enter(const char *name)
{
	if ('@' == name[0]) {
		if (!in_tag) {
			fputs("error: invalid context for ",stderr);
			fputs(name,stderr);
			fputs("\n",stderr);
			exit(1);
		}

		putchar(' ');
		fputs(1 + name,stdout);
		fputs("=\"",stdout);
		return;
	}

	finish_tag();

	if (do_format && root != NULL) {
		putchar('\n');
	}
	print_indent();


	switch (name[0]) {
	case '!':
		if ('\0' != name[1]) {
			fputs("error: invalid comment entry ",stderr);
			fputs(name,stderr);
			fputs("\n",stderr);
			exit(1);
		}
		fputs("<!--",stdout);
		break;
	case '?':
		fputs("<?",stdout);
		fputs(1 + name,stdout);
		fputc(' ',stdout);
		break;
	default:
		fputs("<",stdout);
		fputs(name,stdout);
		in_tag = 1;
		break;
	}
	
	if (do_format) {
		switch(name[0]) {
		case '@': case '?': case '!': break;
		default: indent_level++;
		}
	}
}

static void leave(const char *name)
{
	int was_in_tag = in_tag;

	switch (name[0]) {
	case '@':
		assert(in_tag);
		putchar('"');
		break;
	case '!':
		fputs("-->",stdout);
		break;
	case '?':
		fputs("?>",stdout);
		break;
	default:
		if (do_format) indent_level--;

		if (!do_html && was_in_tag)
			fputs("/>",stdout);
		else {
			const htmlElemDesc *elem = NULL;
			if (do_html) elem = htmlTagLookup((xmlChar *) name);
			finish_tag();
			if (NULL == elem || (!elem->endTag && !elem->empty)) {
				/* Modified: Check text_was_printed flag before adding newline */
				if (do_format && !was_in_tag && !text_was_printed) {
					putchar('\n');
					print_indent();
				}
				fputs("</",stdout);
				fputs(name,stdout);
				putchar('>');
				text_was_printed = 0; /* Reset flag after any closing tag */
			}
		}
		in_tag = 0;
	}
}

static void chars(const char *stuff,const char *context)
{
	switch (context[0]) {
	case '!':
	case '?':
		fputs(stuff,stdout);
		return;

	default:
		finish_tag();
	case '@':
		for (; '\0' != *stuff; ++stuff) switch (*stuff) {
		case '&': fputs("&amp;",stdout);   break;
		case '<': fputs("&lt;",stdout);    break;
		case '>': fputs("&gt;",stdout);    break;
		case '"': fputs("&quot;",stdout);  break;
		default:
#if 1
			putchar(*stuff);
			break;
#else
			if (isprint(*stuff) || isspace(*stuff)) {
				putchar(*stuff);
				break;
			}
			printf("&#%d;",(unsigned char) *stuff);
#endif
		}
	}
	/* New: Set flag if we just printed element text (not attribute text) */
	if (context[0] != '@') text_was_printed = 1;
}

static void release(struct node **ptr)
{
	if (NULL == *ptr) return;
	release(&(*ptr)->child);
	leave((*ptr)->name);
	free((*ptr)->name);
	free(*ptr);
	*ptr = NULL;
}

static void line(char *data)
{
	char *path = data;
	char *content = strchr(data,'=');
	struct node **ptr = &root;
	const char *name = NULL,*context = NULL;

	if (NULL != content) *content++ = '\0';

	if ('/' != path[0]) {
		fputs("warning: invalid line ",stderr);
		fputs(path,stderr);
		fputc('\n',stderr);
		return;
	}

	if (strcmp(path,"/")) {
		name = strtok(path + 1,"/");
		while (name && *ptr && !strcmp(name,(*ptr)->name)) {
			name = strtok(NULL,"/");
			context = (*ptr)->name;
			ptr = &(*ptr)->child;
		}
	}

	if (NULL == name && NULL == *ptr && NULL != content)
		chars("\n",context);

	release(ptr);

	if (NULL == name && NULL == content) {
		if (NULL == context) {
			fputs("error: multiple roots\n",stderr);
			exit(1);
		}
		leave(context);
		enter(context);
	}

	while (NULL != name) {
		*ptr = malloc(sizeof(struct node));
		enter(context = (*ptr)->name = strdup(name));
		ptr = &(*ptr)->child;
		name = strtok(NULL,"/");
	}

	*ptr = NULL;
	if (NULL != content) chars(content,context);
}

int main(int argc,char *argv[])
{
	int num,alloc,len = 0;
	char *buffer = malloc(alloc = 4096);
	const char *name = strrchr(argv[0],'/');
	if (NULL == name) name = argv[0]; else ++name;

	if (!strcmp(name,"2html")) do_html = 1;
	
	int arg;
	while (-1 != (arg = getopt(argc, argv, "f"))) switch(arg) {
	case 'f':
		do_format = 1;
		break;
	case '?':
		fputs("usage: [2xml|2html] [-f] < in > [xml|html]\n",stderr);
		return 2;
	}


	while ((num = read(0,len + buffer,alloc - len)) > 0) {
		char *end = buffer + len + num,*ptr = buffer,*eol;
		while ((eol = memchr(ptr,'\n',end - ptr))) {
			*eol++ = '\0';
			line(ptr);
			ptr = eol;
		}

		memmove(buffer,ptr,(len = end - ptr));
		if (len == alloc) buffer = realloc(buffer,alloc *= 2);
	}

	if (len > 0) {
		buffer[len] = '\0';
		line(buffer);
	}
	/* free(buffer); */
	release(&root);
	putchar('\n');
	return 0;
}