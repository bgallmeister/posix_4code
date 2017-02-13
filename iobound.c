
#define _POSIX_C_SOURCE     199309  /* POSIX 9/1993: .1, .4 */
#include    <unistd.h>

#include    <sys/types.h>
#include    <sys/stat.h>
#include    <fcntl.h>
#include    <stdio.h>
/*
 * This program continuously performs I/O to the device named DEV_FILE.
 * You should make this a serial line or some such;  avoid using disks
 * which may have useful data on them...
 */
#define DEV_FILE "/dev/com1"    /* First serial port on my machine */

main()
{
    int fd, ret;
    char byte;

    fd = open(DEV_FILE, O_WRONLY);
    if (fd < 0) {
        perror(DEV_FILE);
        exit(1);
    }
    alarm(30);
    while (1) {
        ret = write(fd, &byte, sizeof(byte));
        switch (ret) {
            case sizeof(byte):
                break;
            case -1:
                perror("write");
                exit(2);
                break;
            default:
                fprintf(stderr, "Write, ret %d?!?\en", ret);
                exit(3);
                break;
        }
    }
}
