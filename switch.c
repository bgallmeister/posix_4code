
#define _POSIX_C_SOURCE     199309  /* POSIX 9/1993: .1, .4 */
#include    <unistd.h>

#include    <stdio.h>
#include    <sys/types.h>
#include    <signal.h>

/*
 * Measure context switch time using the sched_yield call.
 */
int nswitch = 0;
pid_t chpid;

main()
{
    struct sigaction sa;
    extern void alarm_handler(), child_terminate();

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = alarm_handler;  /* Terminates experiment */

    if (sigaction(SIGALRM, &sa, NULL) < 0) {
        perror("sigaction SIGALRM");
        exit(1);
    }

    sa.sa_handler = child_terminate;    /* Terminates child */
    sigfillset(&sa.sa_mask);/* Take no signals after experiment done */
    if (sigaction(SIGUSR2, &sa, NULL) < 0) {
        perror("sigaction SIGUSR2");
        exit(1);
    }

    /* Should set scheduler here, or use atprio */
    switch (chpid = fork()) {
        case -1:    /* error */
                perror("fork");
                exit(3);
                break;
        default:    /* parent, set alarm and fall through
                 * to common case */
                alarm(60);
        case 0:     /* everybody */
                switcher();
                exit(0);
                break;
    }

    fprintf(stderr, "Unexpected exit from test program!\en");
    exit(4);
}

/* Should never wake up from the sigsuspend--SIGUSR1 is blocked */
switcher()
{
    while (1) {
        sched_yield();
        nswitch++;
    }
}

child_terminate()
{
    printf("%d switches in 60 seconds = %d switch/sec\en", nswitch,
        nswitch / 60);
    exit(0);
}

void
alarm_handler()
{
    printf("%d switches in 60 seconds = %d switch/sec\en", nswitch,
        nswitch / 60);
    kill(chpid, SIGUSR2);
    exit(0);
}
