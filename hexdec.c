#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

#define PROGNAME "hexdec"
#define VERSION "v0.1"

#define SUPPORT_STRING_LITERAL_INJECTION

const unsigned char *skip_hd_header(const unsigned char *line)
{
	const unsigned char *p = line;

	while (*p && isxdigit(*p)) p++;

	if (*p && p[0] == ':' && p[1] == ' ') {
		return p + 2;
	}

	return line;
}

int decode_hex_nibble(int c)
{
	if (c >= '0' && c <= '9') return c - '0';
	else if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	else if (c >= 'A' && c <= 'F') return c - 'A' + 10;

	return 0;
}

//#define CONTINUE_OR_BREAK_ON_INVALID_CHAR break
#define CONTINUE_OR_BREAK_ON_INVALID_CHAR continue

void decode_hex_line(const char *line, int (*putchar_like_callback)(int))
{
	const unsigned char *p;

	for (p = skip_hd_header((unsigned char *)line); p && *p; p++) {
		if (*p == '|' || *p == ';' || *p == '\n' || *p == '\r') break;

		if (isspace(*p)) continue;
		if (isxdigit(*p)) {
			int h1, h2;

			h1 = decode_hex_nibble(*p++);

			if (!isxdigit(*p)) {
				fprintf(stderr, "Premature hex-nibble is found.\n");
				CONTINUE_OR_BREAK_ON_INVALID_CHAR;
			}
			h2 = decode_hex_nibble(*p);

			putchar(h1 * 16 + h2);

			continue;
		}

#ifdef SUPPORT_STRING_LITERAL_INJECTION
		if (*p == '"') {
			int escape = 0;
			int ch;

			while ((ch = *++p) && ch != '\r' && ch != '\n') {
				if (escape) {
					switch (ch) {
					case 'n': ch = '\n'; break;
					case 'r': ch = '\r'; break;
					case 't': ch = '\t'; break;
					case 'v': ch = '\v'; break;
					case 'a': ch = '\a'; break;
					case 'b': ch = '\b'; break;
					case 'f': ch = '\f'; break;
					case '"': ch = '"';  break;
					case '\\': ch = '\\';  break;
					default:
						fprintf(stderr, "Unknown escape sequence.\n");
						break;
					}
					escape = 0;
				} else {
					switch (ch) {
					case '\\':
						escape = 1;
						break;
					case '"':
						goto out;
					default:
						break;
					}
				}
				if (escape) continue;

				putchar(ch);
			}

			fprintf(stderr, "Premature end of string literal is found.\n");
			break;

		out:
			continue;
		}
#endif

		fprintf(stderr, "Invalid character 0x%02x is found.\n", *p);
		CONTINUE_OR_BREAK_ON_INVALID_CHAR;
	}
}

void usage()
{
	fprintf(stderr, "%s %s / The hex(dump) decoder\n\n", PROGNAME, VERSION);
	fprintf(stderr, "Usage: %s [options] [file ...]\n", PROGNAME);
	fprintf(stderr, "  -h: this help\n");

	exit(0);
}


int main(int argc, char *argv[])
{
	int ret = 0;
	int ch;

	while ((ch = getopt(argc, argv, "h")) != -1) {
		switch (ch) {
		case 'h':
		default:
			usage();
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc == 0) {
		static char *argv_stdin[] = {"-"};
		argv = argv_stdin;
		argc = 1;
	}

	for (; argc > 0; argc--, argv++) {
		FILE *fp;
		char buf[1023];

		if (!strcmp(*argv, "-")) {
			fp = stdin;
		} else {
			fp = fopen(*argv, "rt");
			if (fp == NULL) {
				perror(*argv);
				ret = -1;
				continue;
			}
		}

		while (fgets(buf, sizeof(buf), fp) != NULL) {
			decode_hex_line(buf, putchar);
		}

		if (fp != stdin) fclose(fp);
	}

	return ret;
}
