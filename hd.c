#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define PROGNAME "hd"
#define VERSION "v0.2"

#define CHAR_FOLD_SIZE 16
#define BUFFERSIZE 10240

unsigned long fold_size = CHAR_FOLD_SIZE;

int hexdump_formatter(unsigned long start_addr, const char *buf, unsigned long len, void *fprintf_like_callback, void *callback_1st_arg, int flag_print_chars)
{
	int (*callback)(void *, const char *, ...) = (int(*)(void *, const char *, ...))fprintf_like_callback;
	unsigned long i, j;
	unsigned long offset;
	unsigned long count;

	offset = start_addr % fold_size;
	count = (offset + len + fold_size - 1) / fold_size;

	for (i = 0; i < count; i++) {
		callback(callback_1st_arg, "%08lx: ", start_addr);
		start_addr = (start_addr / fold_size) * fold_size;
		for (j = 0; j < fold_size; j++) {
			unsigned long idx = i * fold_size + j - offset;
			if (j % 8 == 0) callback(callback_1st_arg, " ");
			if (idx < len) {
				callback(callback_1st_arg, "%02x ", (unsigned char)buf[idx]);
			} else {
				callback(callback_1st_arg, "   ");
			}
		}
		if (flag_print_chars) {
			callback(callback_1st_arg, " |");
			for (j = 0; j < fold_size; j++) {
				unsigned long idx = i * fold_size + j - offset;
				//if (j % 8 == 0) callback(callback_1st_arg, " ");
				if (idx < len) {
					callback(callback_1st_arg, "%c", (unsigned char)buf[idx] >= 0x20 && (unsigned char)buf[idx] < 0x7fUL ? (unsigned char)buf[idx] : '.');
				} else {
					callback(callback_1st_arg, " ");
				}
			}
			callback(callback_1st_arg, "|");
		}
		callback(callback_1st_arg, "\n");
		start_addr += fold_size;
	}
	return 0;
}

void hexdump(int fd, int flag_print_chars)
{
	char buf[BUFFERSIZE];
	ssize_t len;
	unsigned long pos = 0;

	if (fold_size == 0) fold_size = CHAR_FOLD_SIZE;
	if (fold_size > BUFFERSIZE) {
		fprintf(stderr, "WARNING: Insane fold size.\n");
		fold_size = BUFFERSIZE;
	}

	while ((len = read(fd, buf, sizeof(buf))) > 0) {
		hexdump_formatter(pos, buf, len, fprintf, stdout, flag_print_chars);
		pos += len;
	}

	if (pos) fprintf(stdout, "%08lx: \n", pos);

	return ;
}

void hexencode(int fd, const char *fmtHEAD, const char *fmtHEX, const char *fmtGAP, const char *fmtFOLD, const char *fmtTAIL)
{
	char buf[BUFFERSIZE];
	ssize_t len;
	unsigned long pos = 0;

	while ((len = read(fd, buf, sizeof(buf))) > 0) {
		unsigned char *p = (unsigned char *)buf;
		while (len-- > 0) {
			if (pos) {
				printf("%s", fold_size && pos % fold_size == 0 ? fmtFOLD : fmtGAP);
			} else {
				printf("%s", fmtHEAD);
			}
			printf(fmtHEX, *p++);
			pos++;
		}
		fflush(stdout);
	}

	if (pos) printf("%s\n", fmtTAIL);
	return ;
}

void usage()
{
	fprintf(stderr, "%s %s / The hexdump\n\n", PROGNAME, VERSION);
	fprintf(stderr, "Usage: %s [options] [file ...]\n", PROGNAME);
	fprintf(stderr, "  General\n");
	fprintf(stderr, "    -c: hexdump mode (default)\n");
	fprintf(stderr, "    -q: suppress chars in hexdump mode\n");
	fprintf(stderr, "    -f <N>: fold each N chars. (unlimited = 0, default = %d)\n", CHAR_FOLD_SIZE);
	fprintf(stderr, "            (0 is not supported on hexdump mode)\n");
	fprintf(stderr, "    -h: this help\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  Pre-defined hexencode mode\n");
	fprintf(stderr, "    -S: print hex in simple style   ; equiv. to -X '%%02x' -G ' '\n");
	fprintf(stderr, "    -s: print hex w/ space prefixed ; equiv. to -X ' %%02x'\n");
	fprintf(stderr, "    -u: print url encoded           ; equiv. to -X '%%%%%%02x'\n");
	fprintf(stderr, "    -x: print hex in C style        ; equiv. to -X '0x%%02x' -G ', ' -F ',\\n'\n");
	fprintf(stderr, "    -l: print hex in lisp style     ; equiv. to -X '#x%%02x' -G ' '\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  Design hexencode format by hand\n");
	fprintf(stderr, "    -X <fmt>: set HEX style in printf format\n");
	fprintf(stderr, "    -G <str>: set GAP string between hex octets\n");
	fprintf(stderr, "    -H <str>: set HEADER string for the beginning\n");
	fprintf(stderr, "    -T <str>: set TAIL string for the end\n");
	fprintf(stderr, "    -F <str>: set separator string on each FOLD (default to \"{TAIL}\\n{HEADER}\")\n");
	fprintf(stderr, "\n");

	exit(0);
}

char *strdup_unescape(const char *src)
{
	size_t len = strlen(src);
	unsigned char *dst, *p;
	unsigned char ch;

	dst = p = (unsigned char *)malloc(len + 1);
	if (!dst) return NULL;

	int escape = 0;
	while ((ch = *(unsigned char *)src++)) {
		if (escape) {
			switch (ch) {
			case 'n': ch = '\n'; break;
			case 'r': ch = '\r'; break;
			case 't': ch = '\t'; break;
			case 'v': ch = '\v'; break;
			case 'a': ch = '\a'; break;
			case 'b': ch = '\b'; break;
			case 'f': ch = '\f'; break;
			}
			escape = 0;
		} else if (ch == '\\') {
			escape = 1;
			continue;
		}

		*p++ = ch;
	}
	*p = '\0';

	return (char *)dst;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int ch;
	const char *fmtHEX  = "%02x";
	const char *fmtGAP  = NULL;
	const char *fmtFOLD = NULL;
	const char *fmtHEAD = NULL;
	const char *fmtTAIL = NULL;
	int flag_print_chars = 1;
	int mode_hexdump = 1;

	while ((ch = getopt(argc, argv, "qcSsuxlf:hG:X:F:T:H:")) != -1) {
		switch (ch) {
		case 'q': // suppress chars in hexdump
			flag_print_chars = 0;
			break;
		case 'c': // hexdump
			mode_hexdump = 1;
			break;
		case 'S': // single space
			fmtHEX  = "%02x";
			if (!fmtGAP) fmtGAP = " ";
			mode_hexdump = 0;
			break;
		case 's': // space
			//  12 34 56
			fmtHEX  = " %02x";
			mode_hexdump = 0;
			break;
		case 'u': // url-encode
			// %12%34%56
			fmtHEX  = "%%%02x";
			mode_hexdump = 0;
			break;
		case 'x': // hex
			// 0x12, 0x34, 0x56
			fmtHEX  = "0x%02x";
			if (!fmtGAP) fmtGAP = ", ";
			if (!fmtFOLD) fmtFOLD = ",\n";
			mode_hexdump = 0;
			break;
		case 'l': // lisp
			// #x12 #x34 #x56
			fmtHEX     = "#x%02x";
			if (!fmtGAP) fmtGAP = " ";
			mode_hexdump = 0;
			break;
		case 'f':
			fold_size = atoi(optarg);
			break;
		case 'G':
			fmtGAP  = strdup_unescape(optarg);
			mode_hexdump = 0;
			break;
		case 'X':
			fmtHEX  = strdup_unescape(optarg);
			mode_hexdump = 0;
			break;
		case 'F':
			fmtFOLD = strdup_unescape(optarg);
			mode_hexdump = 0;
			break;
		case 'T':
			fmtTAIL = strdup_unescape(optarg);
			mode_hexdump = 0;
			break;
		case 'H':
			fmtHEAD = strdup_unescape(optarg);
			mode_hexdump = 0;
			break;
		case 'h':
		default:
			usage();
			break;
		}
	}
	argc -= optind;
	argv += optind;

	// post process
	if (fmtGAP  == NULL) fmtGAP = "";
	if (fmtHEAD == NULL) fmtHEAD = "";
	if (fmtTAIL == NULL) fmtTAIL = "";
	if (fmtFOLD == NULL) {
		int len = strlen(fmtHEAD) + 1 + strlen(fmtTAIL);
		char *tmp = malloc(len + 1);
		if (tmp) {
			snprintf(tmp, len + 1, "%s\n%s", fmtTAIL, fmtHEAD);
			fmtFOLD = tmp;
		} else {
			fmtFOLD = "\n";
		}
	}

	if (argc == 0) {
		static char *argv_stdin[] = {"-"};
		argv = argv_stdin;
		argc = 1;
	}

	for (; argc > 0; argc--, argv++) {
		int fd;

		if (!strcmp(*argv, "-")) {
			fd = STDIN_FILENO;
		} else {
			fd = open(*argv, O_RDONLY);
			if (fd < 0) {
				perror(*argv);
				ret = -1;
				continue;
			}
		}

		if (mode_hexdump) {
			hexdump(fd, flag_print_chars);
		} else {
			hexencode(fd, fmtHEAD, fmtHEX , fmtGAP, fmtFOLD, fmtTAIL);
		}
		if (fd != STDIN_FILENO) close(fd);
	}

	return ret;
}
