#ifndef MESSAGE_SLOT_H
#define MESSAGE_SLOT_H

#include <linux/ioctl.h>


#define MAJOR_NUM 245


#define IOCTL_SET_ENC _IOW(MAJOR_NUM, 0, unsigned long)

#define DEVICE_RANGE_NAME "char_dev"
#define BUF_LEN 80
#undef DEVICE_FILE_NAME
#define DEVICE_FILE_NAME "message_slot_dev"
#define SUCCESS 0


#endif
