
#define     _POSIX_C_SOURCE     199309
#include    <unistd.h>
#include    <stdio.h>
#include    <sys/types.h>
#include    <signal.h>
#include    <mqueue.h>

int nreads = 0, nwrites = 0;
pid_t chpid, parentpid;

char *progname;
char *whoami;

#define DEFAULT_NBYTES 4
int nbytes = DEFAULT_NBYTES;

#define MQ_ONE  "/mq_one"
#define MQ_TWO  "/mq_two"

char *buf;

void
usage()
{
    printf("Usage: %s {nbytes} (default nbytes is %d)\en",
        progname, DEFAULT_NBYTES);
    exit(1);
}

main(int argc, char **argv)
{
    struct sigaction sa;
    extern void alarm_handler(int);
    mqd_t m1, m2;
    struct mq_attr ma;

    progname = argv[0];

    if (argc == 2) {
        nbytes = atoi(argv[1]);
    } else if (argc > 2) {
        usage();
    }

    printf("Testing IPC through POSIX.4 mqs using %d-byte sends/recvs\en",
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

    /* Create some message queues */
    ma.mq_flags = 0;    /* No special behavior */
    ma.mq_maxmsg = 1;
    ma.mq_msgsize = nbytes;
    i = mq_unlink(MQ_ONE);  /* Deal with possible leftovers */
    if ((i < 0) && (errno != ENOENT)) {
        perror("mq_unlink");
        exit(1);
    }
    i = mq_unlink(MQ_TWO);  /* Deal with possible leftovers */
    if ((i < 0) && (errno != ENOENT)) {
        perror("mq_unlink");
        exit(1);
    }
    if ((m1 = mq_open(MQ_ONE, O_CREAT|O_EXCL, MODE, &ma)) < 0) {
        perror("mq_open");
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
