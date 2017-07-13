#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include "message_slot.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");

struct message_slot {
    char buffers[4][128];
    int index = -1;
};

typedef struct node {
    int open = 0;
    int id = -1;
    struct message_slot data;
    struct node* next = NULL;
}

static node* head = NULL;



/***************** char device functions *********************/


static int device_open(struct inode *inode, struct file *file) {
    printk("device_open(%p)\n", file);

    int id = file->f_inode->i_ino;

    node* current = head;

    while (current != NULL) {
        if (current->id == id) {
            current->open = 1;
            break;
        } else {
            current = current->next;
        }
    }

    if (current == NULL) {
        if (head == NULL) {
            head = kmalloc(sizeof(struct node), GFP_KERNEL);
            head->id = id;
            head->open = 1;
        } else {
            current = head;
            while (current->next != NULL) current = current->next;
            current->next = kmalloc(sizeof(struct node), GFP_KERNEL);
            current->next->id = id;
            current->next->open = 1;
        }
    }

    return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file) {
    printk("device_release(%p,%p)\n", inode, file);

    int id = file->f_inode->i_ino;

    node* current = head;

    while (current != NULL) {
        if (current->id == id) {
            current->open = 0;
            break;
        } else {
            current = current->next;
        }
    }

    return SUCCESS;
}


static ssize_t device_read(struct file *file, char __user * buffer, size_t length, loff_t * offset) {
    /* read doesnt really do anything (for now) */
    printk("device_read(%p,%d) - operation not supported yet (last written - %s)\n", file, length, Message);

    return -EINVAL;  //  invalid argument error
}


static ssize_t device_write(struct file *file, const char __user * buffer, size_t length, loff_t * offset) {
  int i;
  printk("device_write(%p,%d)\n", file, length);

    int id = file->f_inode->i_ino;

    node* current = head;

    while (current != NULL && current->id != id) current = current->next;

    if (current == NULL) {
        //  TODO file doesnt exist
    } else if (!(current->open)) {
        //  TODO closed
    }

    int index = current->data.index;
    //TODO check index

    for (i = 0; i < 128; i++) {
        if (i < length) {
            if (-EFAULT == get_user(current->data.buffers[index][i], buffer + i)) {
                //TODO handle error
            }
        } else {
            current->data.buffers[index][i] = '0';
        }
    }

    return i;
}

//----------------------------------------------------------------------------
static long device_ioctl(struct file* file, unsigned int ioctl_num, unsigned long ioctl_param) {

  if (IOCTL_SET_ENC == ioctl_num && ioctl_param > -1 && ioctl_param < 4) {
    printk("chardev, ioctl: setting index to %ld\n", ioctl_param);
    node* current = head;
    int id = file->f_inode->i_ino;
    while (current->id != id) current = current->next;
    if (current->open) {
        current->data.index = ioctl_param;
    } else {
        //TODO ERORR
    }
  } else {
    return -EINVAL;
  }

  return SUCCESS;
}

/************** Module Declarations *****************/

/* This structure will hold the functions to be called
 * when a process does something to the device we created */

struct file_operations Fops = {
    .read = device_read,
    .write = device_write,
    .unlocked_ioctl = device_ioctl,
    .open = device_open,
    .release = device_release,
};


static int __init simple_init(void) {
    unsigned int rc = 0;


    rc = register_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME, &Fops);

    if (rc < 0) {
        printk(KERN_ALERT "%s failed with %d\n", "Sorry, registering the character device ", MAJOR_NUM);
        return -1;
    }

    printk("Registeration is a success. The major device number is %d.\n", MAJOR_NUM);

    return 0;
}


static void __exit simple_cleanup(void) {
    node* current head;
    node* next;

    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }

    unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
}

module_init(simple_init);
module_exit(simple_cleanup);
