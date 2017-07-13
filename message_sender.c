#include "message_slot.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char* argv[]) {
    int file_desc, ret_val, index, length;
    char buffer[BUF_SIZE];

    if (argc < 3) {
        printf("Invalid Argument: Not enough arguments\n");
        exit(-1);
    }

    ret_val = sscanf(argv[1], "%d", &index);
    if (ret_val < 1 || index < 0 || index > 3) {
        printf("Invalid Argument: invalid index\n");
        exit(-1);
    }

    file_desc = open("/dev/"DEVICE_FILE_NAME, O_WRONLY);
    if (file_desc < 0) {
        printf("Error: Can't open device file: %s. %s\n", DEVICE_FILE_NAME, strerror(errno));
        exit(-1);
    }

    ret_val = ioctl(file_desc, IOCTL_SET_ENC, index);

    if (ret_val < 0) {
        printf("Error: ioctl_set_msg failed: %d %s\n", ret_val, strerror(errno));
        close(file_desc);
        exit(-1);
    }

    length = snprintf(buffer, BUF_SIZE, "%s", argv[2]);

    if (length < 0) {
        printf("Error: failed to read from input string. %s\n", strerror(errno));
        close(file_desc);
        exit(-1);
    }

    ret_val = write(file_desc, buffer, length > BUF_SIZE ? BUF_SIZE : length);

    if (ret_val < 0) {
        printf("Error: write failed: %d. %s \n", ret_val, strerror(errno));
        close(file_desc);
        exit(-1);
    }
    close(file_desc);

    printf("Succesfully wrote %d chars\n", ret_val);
    return 0;
}
