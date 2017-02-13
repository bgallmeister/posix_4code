
#include    <stdio.h>
#include    <sched.h>

/*
 * Atprio, version 2:  implemented using POSIX.4 standard facilities
 * and virtual scheduling (based at 0, regardless of minima/maxima of
 * implementation).
 *
 * Priority deltas, +-delta, are now supported.
 */

char *progname;

usage() { fprintf(stderr, "Usage: %s {<priority>|[+-]<prio delta>} <command>\en",progname); }

main(int argc, char **argv)
{
    struct sched_param cmd_sched_params;
    int delta, policy;

    progname = argv[0];

    if (argc < 3) {
        usage();
        exit(1);
    } else {
        policy = sched_getscheduler(0);
        if ((policy != SCHED_FIFO) && (policy != SCHED_RR)) {
            fprintf(stderr, "Cannot adjust priority under scheduler %d\en", policy);
            exit(2);
        }
        if ((*argv[1] >= '0') && (*argv[1] <= '9')) {
            /* Explicit priority assignment */
            cmd_sched_params.sched_priority = atoi(argv[1]);
        } else {
            /*
             * Priority delta from current priority.
             * Only works if this process is running
             * SCHED_FIFO or SCHED_RR
             */
            if (*argv[1] == '+') delta = atoi(&argv[1][1]);
            else if (*argv[1] == '-') delta = -atoi(&argv[1][1]);
            else {
                usage();
                exit(1);
            }
            cmd_sched_params.sched_priority =
                vsched_getprio(0) + delta;
        }
        if (vsched_setscheduler(0, policy, &cmd_sched_params) < 0) {
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

