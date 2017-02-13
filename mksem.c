
/*
 * This program makes a named semaphore, and leaves it lying around
 * when it exits.
 */
#define     POSIX_C_SOURCE 199309L

#include    <unistd.h>  /* POSIX et al */
#include    <semaphore.h>   /* sem_* */

main(int argc, char **argv)
{
    sem_t *s;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s semaphore-name\en", argv[0]);
        exit(1);
    }

    /* create semaphore */
    s = sem_open(argv[1], O_CREAT, S_IRWXU, 1); /* value 1==>unlocked */
    if (s == (sem_t *)-1) {
        perror(argv[1]);
        exit(2);
    }

    exit(0);
}

