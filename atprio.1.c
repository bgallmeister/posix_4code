
#include    <stdio.h>
#include    <sched.h>

/*
 * Atprio, version 1:  implemented using POSIX.4 standard facilities
 * and virtual scheduling (based at 0, regardless of minima/maxima of
 * implementation).
 *
 * Simply sets priority to a given number, based at 0.
 */

char *progname;

usage() { fprintf(stderr, "Usage: %s <priority> <command>\en",progname); }

main(int argc, char **argv)
{
    struct sched_param cmd_sched_params;

    progname = argv[0];

    if (argc < 3) {
        usage();
        exit(1);
    } else {
        /* Set de (virtual) priority */
        cmd_sched_params.sched_priority = atoi(argv[1]);
        if (vsched_setscheduler(0, SCHED_FIFO, &cmd_sched_params) < 0) {
            perror("vsched_setscheduler");
            exit(2);
        }
        /* Run de command */
        if (execvp(argv[2], &argv[2]) < 0) {
            perror("execvp");
            exit(3);
        }
    }

}

