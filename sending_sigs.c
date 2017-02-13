
#include    <stdio.h>
#include    <unistd.h>
#include    <sys/types.h>
#include    <signal.h>

/*
 * Measure time sent just for one process to send signals to another process.
 * Avoid all overhead on the child side by having the signal blocked.
 */
int nsigs = 0;
pid_t chpid;

main()
{
    struct sigaction sa;
    sigset_t blockem;
    extern void panic_handler(), alarm_handler(), child_terminate();

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = alarm_handler;  /* Terminates experiment */

    if (sigaction(SIGALRM, &sa, NULL) < 0) {
        perror("sigaction SIGALRM");
        exit(1);
    }

    /* No one should ever _receive_ SIGUSR1--it's blocked.
     * Setting up this handler is just paranoia. */
    sa.sa_handler = panic_handler;
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

    sigemptyset(&blockem);
    sigaddset(&blockem, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &blockem) < 0) {
        perror("sigprocmask");
        exit(2);
    }

    switch (chpid = fork()) {
        case -1:    /* error */
                perror("fork");
                exit(3);
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
    exit(4);
}

/* Should never wake up from the sigsuspend--SIGUSR1 is blocked */
be_a_child()
{
    sigset_t sigset;
    sigfillset(&sigset);
    sigdelset(&sigset, SIGUSR2);    /* Wait for only SIGUSR2 */
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
panic_handler(int sig)
{
    char *signame;
    switch (sig) {
        case SIGUSR1:   signame = "SIGUSR1"; break;
        case SIGUSR2:   signame = "SIGUSR2"; break;
        case SIGALRM:   signame = "SIGALRM"; break;
        default:    signame = "<unknown signal name>"; break;
    }
    printf("ERROR: Child received signal %d (%s)\en",
        sig, signame);
    kill(getppid(), SIGALRM);   /* Terminate experiment */
    exit(1);
}

void
child_terminate()
{
    exit(0);
}

void
alarm_handler()
{
    printf("%d signals sent by parent (%d/sec)\en", nsigs, nsigs/60);
    kill(chpid, SIGUSR2);
    exit(0);
}
