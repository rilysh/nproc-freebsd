#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/cpuset.h>
#include <err.h>
#include <errno.h>
#include <getopt.h>

#if defined (__GNUC__) || defined (__clang__)
# define do_noreturn        __attribute__((noreturn))
# define do_unreachable    __builtin_unreachable()
#else
# define do_noreturn
# define do_unreachable
#endif

/* Options. */
#define OPT_ALL       (1)
#define OPT_IGNORE    (2)
#define OPT_HELP      (3)
#define OPT_VERSION   (4)

static int nproc_affinated_cpus(void)
{
	cpuset_t cpu_mask;

	CPU_ZERO(&cpu_mask);
	if (cpuset_getaffinity(CPU_LEVEL_WHICH, CPU_WHICH_CPUSET, -1,
			       sizeof(cpu_mask), &cpu_mask) == -1)
		err(EXIT_FAILURE, "cpuset_getaffinity()");

        return (CPU_COUNT(&cpu_mask));
}

static int nproc_all_cpus(void)
{
	long ncpus;

	ncpus = sysconf(_SC_NPROCESSORS_CONF);
	if (ncpus == -1)
		err(EXIT_FAILURE, "sysconf()");

	return ((int)ncpus);
}

static int to_int(const char *s)
{
	long ret;
	char *eptr;

	errno = 0;
	ret = strtol(s, &eptr, 10);
	if (errno != 0)
		err(EXIT_FAILURE, "strtol()");
	if (s == eptr)
		errx(EXIT_FAILURE, "no valid integer was provided.");

	/* Checks for overflow and underflow. */
	if (ret > INT_MAX)
		errx(EXIT_FAILURE, "ignore count is too large");
	if (ret < INT_MIN)
		errx(EXIT_FAILURE, "ignore count is too small");

	return ((int)ret);
}

do_noreturn
static void print_help(int status)
{
	FILE *out;

	out = (status == EXIT_SUCCESS) ? stdout : stderr;
	fprintf(out,
		"usage: nproc [--all] [--ignore=count]\n"
		"\tnproc --help\n"
		"\tnproc --version\n");
	exit(status);
}

do_noreturn
static void print_version(void)
{
	fputs("nproc\n", stdout);
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
	int c, cpus, ignore_flag;
        struct option lopts[] = {
		{ "all",     no_argument,        NULL, 1 },
		{ "ignore",  required_argument,  NULL, 2 },
		{ "help",    no_argument,        NULL, 3 },
		{ "version", no_argument,        NULL, 4 },
	};

	ignore_flag = 0;
	while ((c = getopt_long(argc, argv, "", lopts, NULL)) != -1) {
		switch (c) {
		case OPT_ALL:
			fprintf(stdout, "%d\n", nproc_all_cpus());
			goto out_ok;

		case OPT_IGNORE:
			cpus = nproc_affinated_cpus() - to_int(optarg);
			if (cpus < 1)
				cpus = 1;
			ignore_flag = 1;
		        break;

		case OPT_HELP:
			print_help(EXIT_SUCCESS);
			do_unreachable;
			break;

		case OPT_VERSION:
			print_version();
			do_unreachable;
			break;

		case '?':
		        print_help(EXIT_FAILURE);
			do_unreachable;
		default:
			break;
		}
	}

	if (ignore_flag == 0)
		cpus = nproc_affinated_cpus();

	fprintf(stdout, "%d\n", cpus);

out_ok:
	exit(EXIT_SUCCESS);
}
