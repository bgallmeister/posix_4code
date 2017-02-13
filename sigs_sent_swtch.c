
#include    <unistd.h>
#include    <stdio.h>
#include    <sys/types.h>
#include    <signal.h>

int nsigs_sent = 0, nsigs_recv = 0;
pid_t chpid, parentpid;

main()
{
    struct sigaction sa;
    extern void null_handler(), alarm_handler(), child_terminate();
    sigset_t blockem;

    parentpid = getpid();
    sigemptyset(&blockem);
    sigaddset(&blockem, SIGUSR1);
    sigprocmask(SIG_BLOCK, &blockem, NULL);

    sigfillset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = alarm_handler;  /* Terminates experiment */

    if (sigaction(SIGALRM, &sa, NULL) < 0) {
        perror("sigaction SIGALRM");
        exit(1);
    }

    sa.sa_handler = null_handler;       /* Counts signals */
    if (sigaction(SIGUSR1, &sa, NULL) < 0) {
        perror("sigaction SIGUSR1");
        exit(1);
    }

    sa.sa_handler = child_terminate;    /* Terminates child */
    sigfillset(&sa.sa_mask);/* Take no signals after experiment done */
    if (sigaction(SIGUSR2, &sa, NULL) < 0) {
        perror("sigaction SIGUSR2");
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
        if (kill(parentpid, SIGUSR1) < 0) {
            perror("kill");
            return;
        }
        nsigs_sent++;
    }
}

be_the_parent()
{
    sigset_t sigset;

    sigemptyset(&sigset);
    alarm(60);
    while(1) {
        if (kill(chpid, SIGUSR1) < 0) {
            perror("kill");
            return;
        }
        nsigs_sent++;
        sigsuspend(&sigset);
    }
}

void
null_handler()
{
    nsigs_recv++;
}

void
child_terminate()
{
    printf("%d/%d signals sent/received by child (%d sent/sec)\en",
        nsigs_sent, nsigs_recv, nsigs_sent / 60);
    exit(0);
}

void
alarm_handler()
{
    printf("%d/%d signals sent/received by parent (%d sent/sec)\en",
        nsigs_sent, nsigs_recv, nsigs_sent / 60);
    kill(chpid, SIGUSR2);
    exit(0);
}
