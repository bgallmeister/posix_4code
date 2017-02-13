

/*
 * This program benchmarks the bandwidth of shared memory when we use
 * standard POSIX file locks as a means of mutual exclusion.
 * Notice that this test doesn't do any actual synchronization;  there's
 * only one process.  We're benchmarking the case of mutual exclusion.
 * In this case, the majority of accesses are uncontested.
 *
 * Alternatively, you could fork a chile and run two processes at once,
 * benchmarking total throughput.  This would not work well under a real-
 * time scheduler on a uniprocessor, though.
 *
 * Compare this code to code using semaphores.
 */

#define     POSIX_C_SOURCE 199309L

#include    <unistd.h>  /* POSIX et al */
#include    <limits.h>  /* PAGESIZE */
#include    <sys/types.h>   /* fcntl */
#include    <sys/mman.h>    /* shm_open, mmap */
#include    <signal.h>  /* sigaction */

#define SHMNAME     "/my_shm"
#define TIME_PERIOD 60      /* Run test for a minute */

int iterations = 0;
int nbytes;     /* Test, passing 4 bytes of information */

void timer_expired(int called_via_signal)
{
    printf("%d iterations for region of %d bytes\en", iterations, nbytes);
    exit(0);
}

main(int argc, char **argv)
{
    int d;
    char *addr;
    struct sigaction sa;
    struct flock fl;

    if (argc == 2)
        nbytes = atoi(argv[1]); /* Use #bytes passed */
    else
        nbytes = 4; /* Default of 4 bytes (a word on real machines :) */

    sa.sa_handler = timer_expired;
    sa.sa_flags = 0;
    sigemptyst(&sa.sa_sigmask);
    (void)sigaction(SIGALRM, &sa, NULL);

    /* Create shared memory region */
    d = shm_open(SHMNAME, O_RDWR|O_CREAT|O_TRUNC, S_IRWXU);
    ftruncate(d, (off_t)PAGESIZE);  /* Make region PAGESIZE big */
    addr = (char *)mmap(NULL, (size_t)PAGESIZE, PORT_READ|PROT_WRITE,
        MAP_SHARED, d, 0);
    shm_unlink(SHMNAME);    /* So it goes away on exit */

    lockfile = open("lockfile", O_RDWR|O_CREAT|O_TRUNC, S_IRWXU);
    write(lockfile, "A Couple Of Bytes", strlen("A Couple Of Bytes"));
    unlink("lockfile"); /* So it goes away on exit */

    /*
     * Begin test. Repeatedly acquire mutual exclusion, write to area,
     * and release mutual exclusion.
     */
    lockit.l_type = F_WRLCK;
    lockit.l_whence = SEEK_SET;
    lockit.l_start = 0;
    lockit.l_len = 1;

    unlockit = lockit;
    unlockit.l_type = F_UNLCK;

    alarm(TIME_PERIOD);
    while (1) {
        /* acquire parent's lock */
        if (fcntl(lockfile, F_SETLKW, &lockit) < 0) {
            perror("fcntl(F_SETLKW wrlock)");
            exit(1);
        }
        /* store data in shared memory area */
        for (i=0; i<nbytes; i++)
            addr[i] = 'A';
        /* release parent's lock */
        if (fcntl(lockfile, F_SETLKW, &unlockit) < 0) {
            perror("fcntl(F_SETLKW unlock)");
            exit(1);
        }
        iterations++;
    }

}
