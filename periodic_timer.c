
/*
 * This program takes timer interrupts forever.
 * It uses Berkeley-style interval timers,
 * rather than POSIX.4 interval timers.
 * Therefore it doesn't need unistd.h, _POSIX_C_SOURCE, etc.
 */
#include <signal.h>
#include <sys/time.h>
#include <stdio.h>

char *getopt_flags = "t:v"; /* '-t': specify time interval in usec */
extern char *optarg;

#define DEFAULT_USECS 100000    /* 10 Hz */
int verbose = 0;

char *progname;

void
usage()
{
    fprintf(stderr, "Usage: %s {-t usecs}\en", progname);
    return;
}

void timer_intr(int sig)
{
    if (verbose) puts("ouch!");
    return;
}

main(int argc, char **argv)
{
    int c;
    struct itimerval i;
    struct sigaction sa;
    sigset_t allsigs;

    progname = argv[0];

    i.it_interval.tv_sec = 0;
    i.it_interval.tv_usec = DEFAULT_USECS;

    while ((c=getopt(argc, argv, getopt_flags)) != -1) switch (c) {
        case 't':
            i.it_interval.tv_usec = atoi(optarg);
            i.it_interval.tv_sec = 0;
            while (i.it_interval.tv_usec > 1000000) {
                i.it_interval.tv_usec -= 1000000;
                i.it_interval.tv_sec++;
            }
            printf("Time interval: %d sec %d usec\en",
                i.it_interval.tv_sec,
                i.it_interval.tv_usec);
            break;
        case 'v':
            verbose++;
            break;
        default:
            usage();
            exit(1);
    }
    i.it_value = i.it_interval;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = timer_intr;

    if (sigaction(SIGALRM, &sa, NULL) < 0) {
        perror("sigaction");
        exit(2);
    }

    if (setitimer(ITIMER_REAL, &i, NULL) < 0) {
        perror("setitimer");
        exit(3);
    }

    sigemptyset(&allsigs);
    while (1) {
        sigsuspend(&allsigs);
    }
    exit(4);
}
