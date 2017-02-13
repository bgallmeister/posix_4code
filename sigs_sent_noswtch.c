
#include    <stdio.h>
#include    <unistd.h>
#include    <sys/types.h>
#include    <signal.h>

int nsigs = 0;
pid_t chpid;

main()
{
    struct sigaction sa;
    extern void null_handler(), alarm_handler(), child_terminate();

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = alarm_handler;  /* Terminates experiment */

    if (sigaction(SIGALRM, &sa, NULL) < 0) {
        perror("sigaction SIGALRM");
        exit(1);
    }

    sa.sa_handler = null_handler;       /* Counts signals */
    if (sigaction(SIGUSR1, &sa, NULL) < 0) {
        perror("sigaction SIGALRM");
        exit(1);
    }

    sa.sa_handler = child_terminate;    /* Terminates child */
    sigfillset(&sa.sa_mask);/* Take no signals after experiment done */
    if (sigaction(SIGUSR2, &sa, NULL) < 0) {
        perror("sigaction SIGALRM");
        exit(1);
    }


    switch (chpid = fork()) {
        case -1:    /* error */
                perror("fork");
                exit(2);
                break;
        case 0:     /* child */
                be_a_child();
                exit(0);
                break;
        default:    /* parent */
                be_the_parent();
                exit(0);
                break;
    }

    fprintf(stderr, "Unexpected exit from test program!\en");
    exit(3);
}

be_a_child()
{
    sigset_t sigset;
    sigemptyset(&sigset);
    while (1) {
        sigsuspend(&sigset);
    }
}

be_the_parent()
{
    alarm(60);
    while(1) {
        if (kill(chpid, SIGUSR1) < 0) {
            perror("kill");
            return;
        }
        nsigs++;
    }
}

void
null_handler()
{
    nsigs++;
}

void
child_terminate()
{
    printf("%d signals received by child (%d/sec)\en", nsigs, nsigs/60);
    exit(0);
}

void
alarm_handler()
{
    printf("%d signals sent by parent (%d/sec)\en", nsigs, nsigs/60);
    kill(chpid, SIGUSR2);
    exit(0);
}
