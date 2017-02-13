
#include    <stdio.h>
#include    <unistd.h>
#include    <sys/types.h>
#include    <signal.h>

/*
 * Measure time sent just for one process to send signals to itself.
 * Avoid all signal-handling overhead by having the signal blocked.
 */
int nsigs = 0;

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

    /* Should never _receive_ SIGUSR1--it's blocked.
     * Setting up this handler is just paranoia. */
    sa.sa_handler = panic_handler;
    if (sigaction(SIGUSR1, &sa, NULL) < 0) {
        perror("sigaction SIGUSR1");
        exit(1);
    }

    sigemptyset(&blockem);
    sigaddset(&blockem, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &blockem) < 0) {
        perror("sigprocmask");
        exit(2);
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
    printf("ERROR: received signal %d (%s)\en",
        sig, signame);
    exit(5);
}

void
alarm_handler()
{
    printf("%d signals sent (%d/sec)\en", nsigs, nsigs/60);
    exit(0);
}
