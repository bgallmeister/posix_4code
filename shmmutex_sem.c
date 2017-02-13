

/*
 * This program benchmarks the bandwidth of shared memory when we use
 * POSIX.4 semaphores as the mutual exclusion mechanism.
 * In this example, I'm using a named semaphore.
 *
 * Like before, that this test doesn't do any actual synchronization;  there's
 * only one process.  We're benchmarking the case of mutual exclusion.
 * In this case, the majority of accesses are uncontested.
 *
 * Alternatively, you could fork a chile and run two processes at once,
 * benchmarking total throughput.  This would not work well under a real-
 * time scheduler on a uniprocessor, though.
 *
 * Compare this code to code using file locking.
 */

#define     POSIX_C_SOURCE 199309L

#include    <unistd.h>  /* POSIX et al */
#include    <limits.h>  /* PAGESIZE */
#include    <semaphore.h>   /* sem_* */
#include    <sys/mman.h>    /* shm_open, mmap */
#include    <signal.h>  /* sigaction */

#define SHMNAME     "/my_shm"
#define SEMNAME     "/my_sem"
#define TIME_PERIOD 60      /* Run test for a minute */

int iterations = 0;
int nbytes = 4;     /* Test, passing 4 bytes of information */

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
    sem_t *s;

    if (argc == 2)
        nbytes = atoi(argv[1]); /* Use #bytes passed */

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

    /* create semaphore */
    s = sem_open(SEMNAME, O_CREAT, S_IRWXU, 1); /* value 1==>unlocked */
    sem_unlink(SEMNAME);    /* So it goes away on exit */

    /*
     * Begin test. Repeatedly acquire mutual exclusion, write to area,
     * and release mutual exclusion.
     */
    alarm(TIME_PERIOD);
    while (1) {
        /* acquire parent's lock */
        sem_wait(s);
        /* store data in shared memory area */
        for (i=0; i<nbytes; i++)
            addr[i] = 'A';
        /* release parent's lock */
        sem_post(s);
        iterations++;
    }
    /* Semaphore is automatically closed on exit, as is shm */
}
