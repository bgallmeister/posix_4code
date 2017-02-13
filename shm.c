

/* This program, called as "shm_1", creates a shared memory area which
is shared with a process this process creates via fork.  The second
process also does an exec. */

/*
 * Compile with SHM_1 defined to get shm_1 executable.
 * Compile with SHM_2 defined to get shm_2 executable.
 */

#define     POSIX_C_SOURCE 199309L

#include    <unistd.h>  /* POSIX et al */
#include    <limits.h>  /* PAGESIZE */
#include    <sys/mman.h>    /* shm_open, mmap */
#include    <sys/types.h>   /* waitpid */
#include    <sys/wait.h>    /* waitpid */
#include    <signal.h>  /* sigaction */

#define PARENT_SHM_DATA "Parent Wrote This"
#define CHILD_PRE_EXEC_DATA "Child Wrote This Before Exec-ing"
#define CHILD_POST_EXEC_DATA    "Child Wrote This AFTER Exec-ing"

#ifdef SHM_1
#define SHMNAME     "/my_shm"
#define NEW_IMAGE   "shm_2"

void cleanup(int called_via_signal)
{
    (void)shm_unlink(SHMNAME);
    if (called_via_signal)
        exit(3);
}

main()
{
    int d;
    char *addr;
    int chpid;
    int w;
    struct sigaction sa;

    /*
     * In case of premature termination, we want to make sure to
     * clean up the shared memory region.  Hence, the signal handlers.
     */
    sa.sa_handler = cleanup;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    (void)sigaction(SIGINT, &sa, NULL);
    (void)sigaction(SIGBUS, &sa, NULL);
    (void)sigaction(SIGSEGV, &sa, NULL);

    /* Create shared memory region */
    d = shm_open(SHMNAME, O_RDWR|O_CREAT|O_TRUNC, S_IRWXU);
    ftruncate(d, (off_t)PAGESIZE);  /* Make region PAGESIZE big */
    addr = (char *)mmap(NULL, (size_t)PAGESIZE, PORT_READ|PROT_WRITE,
        MAP_SHARED, d, 0);

    /* Put data in the shared memory region */
    printf("Parent stores in SHM: \"%s\"\en", PARENT_SHM_DATA);
    sprintf(addr, PARENT_SHM_DATA);

    /* Create a child process */
    switch (chpid = fork()) {
        case -1:    perror("fork");
                cleanup(0);
                exit(1);
        case 0:     /* child */
                break;
        default:    /* parent;  await child */
                chpid = wait(&w, 0);
                /* Child is done:  see what's in SHM */
                printf("Parent sees in SHM: \"%s\"\en",
                    (char *)addr);
                cleanup(0);
                exit(0);
    }

    /* Code executed by child */
    printf("Child, pre-exec, sees in SHM: \"%s\"\en", addr);
    printf("Child, pre-exec, stores in SHM: \"%s\"\en", CHILD_PRE_EXEC_DATA);
    sprintf(addr, CHILD_PRE_EXEC_DATA);

    /* Exec a new process image */
    execlp(NEW_IMAGE, NEW_IMAGE, SHM_NAME, NULL);
    perror("returned from execlp");
    exit(2);
}
#endif /* SHM_1 */
#ifdef SHM_2
main(int argc, char **argv)
{
    int d;
    char *addr;

    /* Single argument is the name of the shared memory region to map in */
    d = shm_open(SHMNAME, O_RDWR);
    addr = (char *)mmap(NULL, (size_t)PAGESIZE, PORT_READ|PROT_WRITE,
        MAP_SHARED, d, 0);

    printf("Child, after exec, sees: \"%s\"\en", addr);
    printf("Child, post-exec, stores in SHM: \"%s\"\en",
        CHILD_POST_EXEC_DATA);
    sprintf(addr, CHILD_POST_EXEC_DATA);

    exit(0);
}
#endif /* SHM_2 */
