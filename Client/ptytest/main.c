#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdio.h>

static void	set_noecho(int);	/* at the end of this file */
void		loop(int, int);		/* in the file loop.c */

int
main(int argc, char *argv[])
{
	int	fdm, c, ignoreeof, interactive, noecho, verbose;
	pid_t	pid;
	char	*driver, slave_name[20];
	struct termios	orig_termios;
	struct winsize	size;

	interactive = isatty(STDIN_FILENO);
	ignoreeof = 0;
	noecho = 0;
	verbose = 0;
	driver = NULL;

	opterr = 0;		/* don't want getopt() writing to stderr */
	while ( (c = getopt(argc, argv, "d:einv")) != EOF) {
		switch (c) {
		case 'd':		/* driver for stdin/stdout */
			driver = optarg;
			break;

		case 'e':		/* noecho for slave pty's line discipline */
			noecho = 1;
			break;

		case 'i':		/* ignore EOF on standard input */
			ignoreeof = 1;
			break;
		case 'n':		/* not interactive */
			interactive = 0;
			break;

		case 'v':		/* verbose */
			verbose = 1;
			break;

		case '?':
			printf("unrecognized option: -%c", optopt);
		}
	}
	if (optind >= argc)
		printf("usage: pty [ -d driver -einv ] program [ arg ... ]"), exit (1);

	if (interactive) {	/* fetch current termios and window size */
		if (tcgetattr(STDIN_FILENO, &orig_termios) < 0)
			printf("tcgetattr error on stdin");
		if (ioctl(STDIN_FILENO, TIOCGWINSZ, (char *) &size) < 0)
			printf("TIOCGWINSZ error");
		pid = pty_fork(&fdm, slave_name, &orig_termios, &size);

	} else
		pid = pty_fork(&fdm, slave_name, NULL, NULL);

	if (pid < 0)
		printf("fork error");

	else if (pid == 0) {		/* child */
		if (noecho)
			set_noecho(STDIN_FILENO);	/* stdin is slave pty */

		if (execvp(argv[optind], &argv[optind]) < 0)
			printf("can't execute: %s", argv[optind]);
	}

	if (verbose) {
		fprintf(stderr, "slave name = %s\n", slave_name);
		if (driver != NULL)
			fprintf(stderr, "driver = %s\n", driver);
	}

	if (interactive && driver == NULL) {
		//if (tty_raw(STDIN_FILENO) < 0)	/* user's tty to raw mode */
		//	printf("tty_raw error");
		//if (atexit(tty_atexit) < 0)		/* reset user's tty on exit */
		//	err_sys("atexit error");
	}

	loop(fdm, ignoreeof);	/* copies stdin -> ptym, ptym -> stdout */

	exit(0);
}
static void
set_noecho(int fd)		/* turn off echo (for slave pty) */
{
	struct termios	stermios;

	if (tcgetattr(fd, &stermios) < 0)
		printf("tcgetattr error");

	stermios.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
	stermios.c_oflag &= ~(ONLCR);
			/* also turn off NL to CR/NL mapping on output */

	if (tcsetattr(fd, TCSANOW, &stermios) < 0)
		printf("tcsetattr error");
}
