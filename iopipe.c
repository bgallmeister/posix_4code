
#define _POSIX_C_SOURCE 199309L

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>

/*
 * This program copies its standard input to standard output
 * using read and write BUFSIZE bytes at a time.
 */

#define BUFSIZE (64*1024)

char buf[BUFSIZE];

int
main(int argc, char **argv)
{
    ssize_t nbytes;

    fcntl(fileno(stdout), F_GETFL, &flags);
    flags |= O_DSYNC;   /* Just get the data down */
    fcntl(fileno(stdout), F_SETFL, flags);

    while (1) {
        nbytes = read(fileno(stdin), buf, BUFSIZE);
        if (nbytes <= 0)
            break;
        if (write(fileno(stdout), buf, nbytes) != nbytes) {
            perror("write to stdout");
            break;
        }
    }
    exit(0);
}
