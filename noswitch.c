
#define _POSIX_C_SOURCE     199309  /* POSIX 9/1993: .1, .4 */
#include    <unistd.h>

#include    <stdio.h>
#include    <sys/types.h>
#include    <signal.h>
#include    <sched.h>

/*
 * Measure overhead of using the sched_yield call.
 */
int nyield = 0;

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

    switcher(); /* with self ==> no ctxtsw */

    fprintf(stderr, "Unexpected exit from test program!\en");
    exit(4);
}

/* Should never wake up from the sigsuspend--SIGUSR1 is blocked */
switcher()
{
    alarm(60);
    while (1) {
        sched_yield();
        nyield++;
    }
}

void
alarm_handler()
{
    printf("%d yield calls in 60 seconds = %d yield calls/sec\en", nyield,
        nyield / 60);
    exit(0);
}
