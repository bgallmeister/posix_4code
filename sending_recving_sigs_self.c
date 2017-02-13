
#include    <stdio.h>
#include    <unistd.h>
#include    <sys/types.h>
#include    <signal.h>

/*
 * Measure time sent just for one process to send signals to itself
 * AND handle those signals.
 */
int nsigs_sent = 0;
int nsigs_recv = 0;

main()
{
    struct sigaction sa;
    sigset_t blockem;
    extern void null_handler(), alarm_handler();

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = alarm_handler;  /* Terminates experiment */

    if (sigaction(SIGALRM, &sa, NULL) < 0) {
        perror("sigaction SIGALRM");
        exit(1);
    }

    /* Should never _receive_ SIGUSR1--it's blocked.
     * Setting up this handler is just paranoia. */
    sa.sa_handler = null_handler;
    if (sigaction(SIGUSR1, &sa, NULL) < 0) {
        perror("sigaction SIGUSR1");
        exit(1);
    }

    send_sigs_self();

    fprintf(stderr, "Unexpected exit from test program!\en");
    exit(4);
}

send_sigs_self()
{
    pid_t self = getpid();

    alarm(60);
    while(1) {
        if (kill(self, SIGUSR1) < 0) {
            perror("kill");
            return;
        }
        nsigs_sent++;
    }
}

void
null_handler()
{
    nsigs_recv++;
}

void
alarm_handler()
{
    printf("%d signals sent (%d/sec)\en", nsigs_sent, nsigs_sent/60);
    printf("%d signals received (%d/sec)\en", nsigs_recv, nsigs_recv/60);
    exit(0);
}
