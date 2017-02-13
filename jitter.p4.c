
#define     _POSIX_C_SOURCE     199309

#include    <unistd.h>
#include    <sys/types.h>
#include    <time.h>
#include    <sys/signal.h>
#ifdef Lynx
#include    <conf.h>        /* HZ */
#else
/* Most UNIX systems define HZ in param.h. */
#include    <sys/param.h>       /* HZ */
#endif

/*
 * This program measures the jitter using a 100-Hz
 * POSIX.4 interval timer.  SIGRTMIN is the timer signal used.
 *
 * A reasonable real-time system is hardly likely to
 * display any jitter at all at 100 Hz, especially if timestamps
 * are taken only with an accuracy of 100 Hz.  The pedal is more likely
 * to hit the metal at higher rates.
 *
 * The Berkeley-style timer-based jitter program only used the signal as
 * a means to terminate a call to sigsuspend;  nothing actually happened in
 * that signal handler.  In contrast, this program performs the jitter
 * calculation in the signal handler (just to add a little variety).
 */

#define DEFAULT_SECS 0
#define DEFAULT_NSECS (1000000000 / HZ) /* 100 times a second */

#define TIMEBUF_MAX 200
struct timespec start_time, end_time, ta[TIMEBUF_MAX];
int this, prev;
long secs, nsecs;


#define JITTERBUF_MAX 100
struct {
    int next;
    struct timespec j[JITTERBUF_MAX];
} jbuf;
int nsig;

extern void ctrlc(int);
extern void timer_expiration(int, siginfo_t *, void *);

main(int argc, char **argv)
{
    sigset_t block_these, pause_mask;
    struct sigaction s;
    struct itimerspec interval;
    timer_t tid;
    struct sigevent notification;

    /*
     * I assume this program is externally run at the highest priority
     * in the system.  A program like "atprio" can be used for this
     * purpose.
     */

    /* Lock all process memory down */
    if (mlockall(MCL_CURRENT|MCL_FUTURE) < 0) {
        perror("mlockall");
        exit(1);
    }
    sigemptyset(&block_these);
    sigaddset(&block_these, SIGRTMIN);
    sigprocmask(SIG_BLOCK, &block_these, &pause_mask);
    if (sigismember(&pause_mask, SIGRTMIN)) {
        printf("ALRM was part of previous mask\en");
        sigdelset(&pause_mask, SIGRTMIN);
    }
    /* Handler for SIGINT */
    sigemptyset(&s.sa_mask);
    sigaddset(&s.sa_mask, SIGRTMIN);
    s.sa_flags = 0L;
    s.sa_handler = ctrlc;
    if (sigaction(SIGINT, &s, NULL) < 0) {
        perror("sigaction SIGINT");
        exit(1);
    }
    /* Handler for RT signal SIGRTMIN */
    sigemptyset(&s.sa_mask);
    sigaddset(&s.sa_mask, SIGINT);
    s.sa_flags = SA_SIGINFO;
    s.sa_sigaction = timer_expiration;
    if (sigaction(SIGRTMIN, &s, NULL) < 0) {
        perror("sigaction SIGRTMIN");
        exit(1);
    }

    secs = DEFAULT_SECS;
    nsecs = DEFAULT_NSECS;
    /* Assure nsecs is modulo HZ (paranoia) */
    nsecs -= nsecs % HZ;

    interval.it_value.tv_sec = secs;
    interval.it_value.tv_nsec = nsecs;
    interval.it_interval.tv_sec = secs;
    interval.it_interval.tv_nsec = nsecs;

    jbuf.next = 0;
    nsig = 0;
    prev = -1;
    this = 0;

    clock_gettime(CLOCK_REALTIME, &start_time);
    notification.sigev_notify = SIGEV_SIGNAL;
    notification.sigev_signo = SIGRTMIN;
    notification.sigev_value.sival_ptr = (void *)&tid;
    if (timer_create(CLOCK_REALTIME, &notification, &tid) < 0) {
        perror("timer_create");
        exit(1);
    }
    timer_settime(tid, 0, &interval, NULL);
    while (1) {
        sigsuspend(&pause_mask);
    }

}

void ctrlc(int sig)
{
    int i;
    int total_sec, total_nsec;
    float totaltime;
    struct timespec jmax;
    int start_sec;

    gettimeofday(&end_time, NULL);
    total_sec = end_time.tv_sec - start_time.tv_sec;
    total_nsec = end_time.tv_nsec - start_time.tv_nsec;
    totaltime = (float)total_sec * 1000000000. + (float)total_nsec;
    if (total_nsec < 0) {
        total_sec++;
        total_nsec += 1000000000;
    }
    printf("Control-C\en");
    printf("%d signals in %d sec %d nsec = 1 signal every %f nsec\en",
        nsig, total_sec, total_nsec,
        totaltime / (float)nsig);
    jmax.tv_sec = jmax.tv_nsec = 0;
    totaltime = 0.;
    for (i=0; i<jbuf.next; i++) {
        if ((abs(jbuf.j[i].tv_sec) > jmax.tv_sec) ||
            (abs(jbuf.j[i].tv_nsec) > jmax.tv_nsec)) {
                jmax.tv_sec = abs(jbuf.j[i].tv_sec);
                jmax.tv_nsec = abs(jbuf.j[i].tv_nsec);
        }
        totaltime +=
            (((float)abs(jbuf.j[i].tv_sec)) * 1000000000.) +
             ((float)abs(jbuf.j[i].tv_nsec));
    }
    printf("Max jitter: %d nsec\en", jmax.tv_sec * 1000000000 + jmax.tv_nsec);
    /* Jitter wasn't measured on the first signal */
    printf("Average jitter: %f nsec\en", totaltime / (float)(nsig-1));
    if (jbuf.next) {
        /* There was jitter */
        start_sec = ta[0].tv_sec;
        for (i=0; i<prev; i++)
            printf("%-5d %-7d\en",
            ta[i].tv_sec - start_sec,
            ta[i].tv_nsec);
    }

    exit(0);
}


void timer_expiration(int sig, siginfo_t *info, void *extra)
{
    nsig++;
    /* Calculate jitter: difference between the actual
     * time and the expected time */
    clock_gettime(CLOCK_REALTIME, &ta[this]);
    if (prev >= 0) {
        if ((ta[this].tv_sec !=
            ta[prev].tv_sec + secs) ||
            (ta[this].tv_nsec !=
            ta[prev].tv_nsec + nsecs)) {

            /* There seems to have been jitter.  Verify. */

            if ((ta[this].tv_sec==ta[prev].tv_sec + 1) &&
                (ta[this].tv_nsec == 0))
                /* No jitter;  the seconds just clicked over. */
                goto skip;

            /* Calculate the amount of jitter. */
            jbuf.j[jbuf.next].tv_sec =
                ta[this].tv_sec - ta[prev].tv_sec;
            jbuf.j[jbuf.next].tv_nsec =
                ta[this].tv_nsec - ta[prev].tv_nsec;
            jbuf.next++;
            if (jbuf.next == JITTERBUF_MAX) {
                ctrlc(0);   /* Terminate */
            }
        }
        skip:
            prev = this;
            this++;
            if (this == TIMEBUF_MAX)
                this = 0;
    }
}
