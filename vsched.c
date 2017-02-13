
#include    <sched.h>

/*
 * This is one implementation of the virtual scheduling interface I mentioned
 * in the scheduling chapter.  It allows you to use a range of priorities
 * based at 0, regardless of the underlying implementation's priority numbers.
 * This code is used in all three of the versions of "atprio".
 *
 * Note: this might get you in trouble if your system supported different
 * schedulers in different priority ranges.  Your virtual priority 0 for
 * SCHED_FIFO would not compete equally with your virtual priority 0 for
 * SCHED_RR, e.g., if the lowest underlying priorities were not the same.
 */
static int sched_rr_min, sched_rr_max;
static int sched_fifo_min, sched_fifo_max;

static int vsched_initialized = 0;

static void
vsched_init()
{
    sched_rr_min = sched_get_priority_min(SCHED_RR);
    sched_rr_max = sched_get_priority_max(SCHED_RR);
    sched_fifo_min = sched_get_priority_min(SCHED_FIFO);
    sched_fifo_max = sched_get_priority_max(SCHED_FIFO);
}


int
vsched_setscheduler(
    pid_t pid,                  /* Process to affect */
    int vsched_policy,              /* Policy to set */
    const struct sched_param *vsched_param)     /* Parameters to set */
{
    struct sched_param tmp;

    if (! vsched_initialized)
        vsched_init();

    tmp = *vsched_param;
    switch (vsched_policy) {
    case SCHED_FIFO:
        tmp.sched_priority += sched_fifo_min;
        break;
    case SCHED_RR:
        tmp.sched_priority += sched_rr_min;
        break;
    default:
        break;  /* Do nothing */
    }
    return (sched_setscheduler(pid, vsched_policy, &tmp));
}


int
vsched_setprio(
    pid_t pid,                  /* Process to affect */
    int vsched_prio)                /* Priority to set */
{
    struct sched_param tmp;
    int sched_policy = sched_getscheduler(pid);

    if (! vsched_initialized)
        vsched_init();

    tmp.sched_priority = vsched_prio;
    switch (sched_policy) {
    case SCHED_FIFO:
        tmp.sched_priority += sched_fifo_min;
        break;
    case SCHED_RR:
        tmp.sched_priority += sched_rr_min;
        break;
    default:
        break;  /* Do nothing; function below will return error */
    }
    return (sched_setscheduler(pid, sched_policy, &tmp));
}


int
vsched_getprio(
    pid_t pid)      /* Process whose priority is desired */
{
    struct sched_param tmp;
    int sched_policy;

    if (! vsched_initialized)
        vsched_init();

    if ((sched_policy=sched_getscheduler(pid)) < 0) {
        return -1;
    }
    if (sched_getparam(pid, &tmp) < 0) {
        return -1;
    }
    switch (sched_policy) {
    case SCHED_FIFO:
        return tmp.sched_priority - sched_fifo_min;
    case SCHED_RR:
        return tmp.sched_priority - sched_rr_min;
    default:
        return -1;  /* Invalid virtual priority */
    }
}
