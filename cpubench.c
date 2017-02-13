
#define _POSIX_C_SOURCE     199309  /* POSIX 9/1993: .1, .4 */
#include    <unistd.h>

#include    <stdio.h>
#include    <sys/types.h>
#include    <signal.h>

/*
 * Simple CPU usage benchmark.
 *
 * You are not likely to see any difference at all when running this
 * concurrently with a timer that goes off at 100, 1000, or even 5000 Hz.
 * The reason is that the majority of systems don't support such high
 * resolutions, and will quietly trim back your timer interval to the
 * maximum supported by the system, usually on the order of 100 Hz.
 * So, setting an interval timer higher than this maximum resolution
 * does not produce any further degradation of background computation--
 * because the timer just don't go no faster!
 */

#define INTERVAL_SECS 10

int niter;

main()
{
    struct sigaction sa;
    extern void alarm_handler();

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = alarm_handler;  /* Terminates experiment */

    if (sigaction(SIGALRM, &sa, NULL) < 0) {
        perror("sigaction SIGALRM");
        exit(1);
    }

    alarm(INTERVAL_SECS);

    while (1)
        niter++;
}

void
alarm_handler()
{
    printf("%d iteractions in %d sec = %d iter/sec\en",
        niter, INTERVAL_SECS, niter / INTERVAL_SECS);
    exit(0);
}
