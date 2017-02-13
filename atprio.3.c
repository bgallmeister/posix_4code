
#include    <stdio.h>
#include    <sched.h>

/*
 * Atprio, version 3:  implemented using POSIX.4 standard facilities
 * and virtual scheduling (based at 0), supporting priority adjustment
 * and the ability to set/adjust the scheduling priority of the calling
 * process.
 */

char *progname;

usage() { fprintf(stderr, "Usage: %s {<priority>|[+-]<prio delta>} <command>\en",progname); }

main(int argc, char **argv)
{
    struct sched_param cmd_sched_params;
    int delta, policy;
    int pid_to_affect = 0;  /* self */
    char **exec_this;

    progname = argv[0];

    /* Figure out who to affect--us or our parent--and what to execute. */
    if (argc == 2) {
        /* atprio 3 or atprio +-7: affect caller's priority */
        pid_to_affect = getppid();  /* Parent PID */
        exec_this = NULL;
    } else if (argc >= 3) {
        /* atprio 16 ls -lR: run command at a priority */
        exec_this = &argv[2];
    }

    policy = sched_getscheduler(pid_to_affect);
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
            vsched_getprio(pid_to_affect) + delta;
    }
    if (vsched_setscheduler(pid_to_affect, policy, &cmd_sched_params) < 0) {
        perror("vsched_setscheduler");
        exit(2);
    }
    /* Run de command, if dere is one */
    if (exec_this != NULL) {
        if (execvp(*exec_this, exec_this) < 0) {
            perror("execvp");
            exit(3);
        }
    }
    exit(0);
}
