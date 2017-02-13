
/*
 * This program takes timer interrupts forever.
 * It uses POSIX.4 interval timers.
 */
#define _POSIX_C_SOURCE 199309
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <stdio.h>

char *getopt_flags = "t:v"; /* '-t': specify time interval in usec */
extern char *optarg;

#define DEFAULT_NSECS   100000000   /* 10 Hz */

int verbose = 0;

char *progname;

void
usage()
{
    fprintf(stderr, "Usage: %s {-t nsecs} {-v}\en", progname);
    return;
}

void timer_intr(int sig, siginfo_t *extra, void *cruft)
{
    if (verbose) {
        if (extra != NULL) {
            printf("sig %d code %d val (ptr) %x overrun %d\en",
                extra->si_signo,
                extra->si_code,
                extra->si_value.sival_ptr,
                timer_getoverrun(*(timer_t *)
                    extra->si_value.sival_ptr));
        } else {
            printf("No extra data\en");
        }
    }
    return;
}

timer_t mytimer;

main(int argc, char **argv)
{
    int c;
    struct itimerspec i;
    struct sigaction sa;
    sigset_t allsigs;
    struct sigevent timer_event;

    progname = argv[0];

    i.it_interval.tv_sec = 0;
    i.it_interval.tv_nsec = DEFAULT_NSECS;

    while ((c=getopt(argc, argv, getopt_flags)) != -1) switch (c) {
        case 't':
            i.it_interval.tv_nsec = atoi(optarg);
            i.it_interval.tv_sec = 0;
            while (i.it_interval.tv_nsec > 1000000000) {
                i.it_interval.tv_nsec -= 1000000000;
                i.it_interval.tv_sec++;
            }
            printf("Time interval: %d sec %d nsec\en",
                i.it_interval.tv_sec,
                i.it_interval.tv_nsec);
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
    sa.sa_flags = SA_SIGINFO;   /* Real-Time signal */
    sa.sa_sigaction = timer_intr;

    if (sigaction(SIGRTMIN, &sa, NULL) < 0) {
        perror("sigaction");
        exit(2);
    }

    /* Create a timer based upon the CLOCK_REALTIME clock */

    /*
     * This describes the asynchronous notification to be posted
     * upon this timer's expiration:
     * - use signals (not that there's any other alternative at present)
     * - send SIGRTMIN
     * - send extra data consisting of a pointer back to the timer ID.
     */
    timer_event.sigev_notify = SIGEV_SIGNAL;
    timer_event.sigev_signo = SIGRTMIN;
    timer_event.sigev_value.sival_ptr = (void *)&mytimer;

    if (timer_create(CLOCK_REALTIME, &timer_event, &mytimer) < 0) {
        perror("timer_create");
        exit(5);
    }
    if (verbose) printf("Timer id at location %x\en", &mytimer);
    if (timer_settime(mytimer, 0, &i, NULL) < 0) {
        perror("setitimer");
        exit(3);
    }

    sigemptyset(&allsigs);
    while (1) {
        sigsuspend(&allsigs);
    }
    exit(4);
}

