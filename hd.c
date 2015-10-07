#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define PROGNAME "hd"
#define VERSION "v0.1"

#define CHAR_FOLD_SIZE 16

int hexdump_formatter(unsigned long start_addr, const char *buf, unsigned long len, void *fprintf_like_callback, void *callback_1st_arg, int flag_print_chars)
{
	int (*callback)(void *, const char *, ...) = (int(*)(void *, const char *, ...))fprintf_like_callback;
	unsigned long i, j;
	unsigned long offset;
	unsigned long count;

	offset = start_addr % CHAR_FOLD_SIZE;
	count = (offset + len + CHAR_FOLD_SIZE - 1) / CHAR_FOLD_SIZE;

	for (i = 0; i < count; i++) {
		callback(callback_1st_arg, "%08lx: ", start_addr);
		start_addr = (start_addr / CHAR_FOLD_SIZE) * CHAR_FOLD_SIZE;
		for (j = 0; j < CHAR_FOLD_SIZE; j++) {
			unsigned long idx = i * CHAR_FOLD_SIZE + j - offset;
			if (j == 8) callback(callback_1st_arg, " ");
			if (idx < len) {
				callback(callback_1st_arg, "%02x ", (unsigned char)buf[idx]);
			} else {
				callback(callback_1st_arg, "   ");
			}
		}
		if (flag_print_chars) {
			callback(callback_1st_arg, " |");
			for (j = 0; j < CHAR_FOLD_SIZE; j++) {
				unsigned long idx = i * CHAR_FOLD_SIZE + j - offset;
				//if (j == 8) callback(callback_1st_arg, " ");
				if (idx < len) {
					callback(callback_1st_arg, "%c", (unsigned char)buf[idx] >= 0x20 && (unsigned char)buf[idx] < 0x7fUL ? (unsigned char)buf[idx] : '.');
				} else {
					callback(callback_1st_arg, " ");
				}
			}
			callback(callback_1st_arg, "|");
		}
		callback(callback_1st_arg, "\n");
		start_addr += CHAR_FOLD_SIZE;
	}
	return 0;
}

void process_file(int fd, const char *fmt, int flag_print_chars)
{
	char buf[10240];
	ssize_t len;
	unsigned long pos = 0;

	while ((len = read(fd, buf, sizeof(buf))) > 0) {
		if (fmt) {
			unsigned char *p = (unsigned char *)buf;
			while (len-- > 0) {
				printf(fmt, *p++);
				pos++;
				if (pos % CHAR_FOLD_SIZE == 0) putchar('\n');
			}
			fflush(stdout);
		} else {
			hexdump_formatter(pos, buf, len, fprintf, stdout, flag_print_chars);
			pos += len;
		}
	}

	if (pos) {
		if (fmt) printf("\n");
		else fprintf(stdout, "%08lx: \n", pos);
	}
	return ;
}

void usage()
{
	fprintf(stderr, "%s %s / The hexdump\n\n", PROGNAME, VERSION);
	fprintf(stderr, "Usage: %s [options] [file ...]\n", PROGNAME);
	fprintf(stderr, "  -q: suppress chars in hexdump\n");
	fprintf(stderr, "  -c: print hexdump (default)\n");
	fprintf(stderr, "  -s: print space separated \" 12 34 56\"\n");
	fprintf(stderr, "  -u: print url encoded     \"%%12%%34%%56\"\n");
	fprintf(stderr, "  -x: print hex encoded     \"0x12, 0x34, 0x56, \"\n");
	fprintf(stderr, "  -h: this help\n");

	exit(0);
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int ch;
	char *fmt = NULL;
	int flag_print_chars = 1;

	while ((ch = getopt(argc, argv, "qcsuxh")) != -1) {
		switch (ch) {
		case 'q': // suppress chars in hexdump
			flag_print_chars = 0;
			break;
		case 'c': // hexdump
			fmt = NULL;
			break;
		case 's': // space
			//  12 34 56
			fmt = " %02x";
			break;
		case 'u': // url-encode
			// %12%34%56
			fmt = "%%%02x";
			break;
		case 'x': // hex
			// 0x12, 0x34, 0x56, 
			fmt = "0x%02x, ";
			break;
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
		process_file(fd, fmt, flag_print_chars);
		if (fd != STDIN_FILENO) close(fd);
	}

	return ret;
}
