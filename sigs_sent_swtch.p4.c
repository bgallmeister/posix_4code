
#define     _POSIX_C_SOURCE 199309

#include    <unistd.h>
#include    <stdio.h>
#include    <unistd.h>
#include    <sys/types.h>
#include    <signal.h>

int nsigs_sent = 0, nsigs_recv = 0;
pid_t chpid, parentpid;

/* Send signals between processes using POSIX.4 real-time queued signals */

main()
{
    struct sigaction sa;
    extern void alarm_handler(), child_terminate();
    extern void handler(int, siginfo_t *, void *);
    sigset_t blockem;

    parentpid = getpid();
    sigemptyset(&blockem);
    sigaddset(&blockem, SIGRTMIN);
    sigprocmask(SIG_BLOCK, &blockem, NULL);

    sigfillset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = alarm_handler;  /* Terminates experiment */

    if (sigaction(SIGALRM, &sa, NULL) < 0) {
        perror("sigaction SIGALRM");
        exit(1);
    }

    sa.sa_handler = child_terminate;    /* Terminates child */
    if (sigaction(SIGUSR2, &sa, NULL) < 0) {
        perror("sigaction SIGUSR2");
        exit(1);
    }

    sigemptyset(&sa.sa_mask);   /* No particular signal blockage */
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;      /* Counts signals */
    if (sigaction(SIGRTMIN, &sa, NULL) < 0) {
        perror("sigaction SIGRTMIN");
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
    union sigval val;

    sigemptyset(&sigset);
    val.sival_int = 0;
    while (1) {
        sigsuspend(&sigset);
        if (sigqueue(parentpid, SIGRTMIN, val) < 0) {
            perror("sigqueue");
            return;
        }
        nsigs_sent++;
        val.sival_int++;    /* Send different extra information */
    }
}

be_the_parent()
{
    sigset_t sigset;
    union sigval val;

    sigemptyset(&sigset);
    alarm(60);
    val.sival_int = 0;
    while(1) {
        if (sigqueue(chpid, SIGRTMIN, val) < 0) {
            perror("sigqueue");
            return;
        }
        nsigs_sent++;
        sigsuspend(&sigset);
        val.sival_int++;    /* Send different information */
    }
}

/*
 * The handler here does the same as does the handler for a plain old signal.
 * Remember, though, that the handler is _receiving_ extra information in
 * the info parameter.  In this test, that information is discarded.
 * However, in most applications the data will probably be used.
 */
void
handler(int signo, siginfo_t *info, void *extra)
{
    nsigs_recv++;
}

void
child_terminate(int signo)
{
    printf("%d/%d signals sent/received by child (%d sent/sec)\en",
        nsigs_sent, nsigs_recv, nsigs_sent / 60);
    exit(0);
}

void
alarm_handler(int signo)
{
    printf("%d/%d signals sent/received by parent (%d sent/sec)\en",
        nsigs_sent, nsigs_recv, nsigs_sent / 60);
    kill(chpid, SIGUSR2);
    exit(0);
}
