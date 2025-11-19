#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

int do_first = 0;
const char *quote = "";
const char *delimiter = ",";
const char *path_prefix = "file/record";
const char *namespace_uri = NULL; // New: To store the namespace URI

int num_fields = 0;
int recno = 0;
const char **field_names = NULL;

int isvalid(const char *begin,const char *end) {
	return begin != end && isalnum(*begin);
}

void field(int num,const char *begin,const char *end) {
	while (begin != end && isspace(begin[0])) ++begin;
	while (end != begin && isspace(end[-1])) --end;

	if (do_first) {
		const char *name;
		char *ptr;

		if (num >= num_fields)
			field_names = realloc(field_names,
				sizeof(*field_names) * (1 + num));

		while (num >= num_fields)
			field_names[num_fields++] = NULL;

		name = ptr = malloc(7 + end - begin);

		if (isvalid(begin,end))
			do *ptr++ = *begin++;
			while (isvalid(begin,end));
		else {
			strcpy(ptr,"field");
			ptr += 5;
		}

		while (end != begin) {
			while (end != begin && !isvalid(begin,end)) ++begin;
			if (isvalid(begin,end)) {
				*ptr++ = '-';
				do *ptr++ = *begin++;
				while (isvalid(begin,end));
			}
		}

		*ptr++ = '\0';
		field_names[num] = name;
	} else {
		fprintf(stdout,"/%s/",path_prefix);
		if (num < num_fields && NULL != field_names[num])
			fputs(field_names[num],stdout);
		else
			printf("field%d",num);
		printf("=%.*s\n",(int)(end - begin),begin);
	}
}

void line(const char *l) {
	int num = 0;
	if (!do_first) {
		fprintf(stdout,"/%s\n",path_prefix);
		fprintf(stdout,"/%s/@num=%d\n",path_prefix, recno++);
	}
	for (;;) {
		if (NULL != strchr(quote,*l)) {
			size_t len = strcspn(++l,quote);
			field(num++,l,len + l);
			l += len;
			if (NULL != strchr(quote,*l)) ++l;
		} else {
			size_t len = strcspn(l,delimiter);
			field(num++,l,len + l);
			l += len;
		}

		if ('\0' == *l) break;
		if (NULL != strchr(delimiter,*l)) ++l;
	}
	do_first = 0;
}

int main(int argc,char *argv[]) {
	int arg,num,alloc,len = 0;
	char *buffer = malloc(alloc = 4096);

	// Modified: Added 'n:' to getopt string
	while (EOF != (arg = getopt(argc,argv,"fq:d:p:n:"))) switch (arg) {
	case 'f': ++do_first; break;
	case 'q': quote = optarg; break;
	case 'd': delimiter = optarg; break;
	case 'p': path_prefix = optarg; break;
	case 'n': namespace_uri = optarg; break; // New: Handle the -n option
	case '?':
		// Modified: Updated usage message
		fputs("usage: csv2 [-f] [-q quote] [-d comma] [-p path] [-n namespace] < csv > out\n",
		      stderr);
		return 2;
	}

	if (optind != argc) {
		fprintf(stderr,"unexpected argument: \"%s\"\n",argv[optind]);
		return 2;
	}

    // New: Logic to print the namespace attribute line
    if (namespace_uri != NULL) {
        // Find the length of the first component of the path_prefix
        size_t root_len = strcspn(path_prefix, "/");
        if (root_len > 0) {
            // Print the attribute line for the root element
            fprintf(stdout, "/%.*s/@xmlns=%s\n", (int)root_len, path_prefix, namespace_uri);
        }
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
	return 0;
}
