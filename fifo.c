
#define     _POSIX_C_SOURCE     199309
#include    <unistd.h>
#include    <stdio.h>
#include    <sys/types.h>
#include    <signal.h>

int nreads = 0, nwrites = 0;
pid_t chpid, parentpid;

char *progname;
char *whoami;

#define DEFAULT_NBYTES 4
int nbytes = DEFAULT_NBYTES;

char *buf;

void
usage()
{
    printf("Usage: %s {nbytes} (default nbytes is %d)\en",
        progname, DEFAULT_NBYTES);
    exit(1);
}

/* Descriptive array indices for pipes */
#define WRITE_END   1
#define READ_END    0

main(int argc, char **argv)
{
    struct sigaction sa;
    extern void alarm_handler(int);
    int pe1[2], pe2[2]; /* Pipeage */

    progname = argv[0];

    if (argc == 2) {
        nbytes = atoi(argv[1]);
    } else if (argc > 2) {
        usage();
    }

    printf("Testing IPC through pipes using %d-byte reads/writes\en",
        nbytes);

    if ((buf = (char *)malloc(nbytes)) == NULL) {
        perror("malloc");
        exit(1);
    }

    /* Set up signals used for terminating the experiment */
    sigfillset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = alarm_handler;  /* Terminates experiment */
    if (sigaction(SIGALRM, &sa, NULL) < 0) {
        perror("sigaction SIGALRM");
        exit(1);
    }

    /* Create some pipes */
    if (pipe(pe1) < 0) {
        perror("pipe");
        exit(1);
    }
    if (pipe(pe2) < 0) {
        perror("pipe");
        exit(1);
    }

    /* Duplicate the process */
    switch (chpid = fork()) {
        case -1:    /* error */
                perror("fork");
                exit(2);
                break;
        case 0:     /* child */
                whoami = "child";
                be_a_child(pe1[WRITE_END], pe2[READ_END]);
                exit(0);
                break;
        default:    /* parent */
                whoami = "parent";
                be_the_parent(pe2[WRITE_END], pe1[READ_END]);
                exit(0);
                break;
    }

    fprintf(stderr, "Unexpected exit from test program!\en");
    exit(3);
}

be_a_child(int write_this, int read_this)
{
    int ret;

    while (1) {
        if ((ret=read(read_this, buf, nbytes)) != nbytes) {
        printf("Returned %d bytes trying to read %d\en", ret, nbytes);
            perror("child read from pipe");
            exit(1);
        }
        nreads++;
        if (write(write_this, buf, nbytes) != nbytes) {
            perror("child write to pipe");
            exit(1);
        }
        nwrites++;
    }
}

be_the_parent(int write_this, int read_this)
{
    alarm(60);
    while (1) {
        if (write(write_this, buf, nbytes) != nbytes) {
            perror("parent write to pipe");
            exit(1);
        }
        nwrites++;
        if (read(read_this, buf, nbytes) != nbytes) {
            perror("parent read from pipe");
            exit(1);
        }
        nreads++;
    }
}

void alarm_handler(int signo)
{
    printf("%d/%d reads/writes (%d bytes each) by %s (%d bytes sent/sec)\en",
        nreads, nwrites, nbytes, whoami, (nwrites * nbytes) / 60);
    if (getpid() != chpid)  /* Parent--kill child too */
        kill(chpid, SIGALRM);
    exit(0);
    
}

