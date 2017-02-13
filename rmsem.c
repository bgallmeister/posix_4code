
/*
 * This program removes a named semaphore.
 */
#define     POSIX_C_SOURCE 199309L

#include    <unistd.h>  /* POSIX et al */
#include    <semaphore.h>   /* sem_* */

main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s semaphore-name\en", argv[0]);
        exit(1);
    }

    /* remove semaphore */
    if (sem_unlink(argv[1]) < 0) {
        perror(argv[1]);
        exit(2);
    }

    exit(0);
}
