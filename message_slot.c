#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include "message_slot.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");

struct message_slot {
    char buffers[4][128];
    int index;
};

typedef struct node {
    int open;
    int id;
    struct message_slot data;
    struct node* next;
} node_t;

static node_t* head = NULL;



/***************** char device functions *********************/


static int device_open(struct inode *inode, struct file *file) {
    int id;
    node_t* current_node;


    printk("device_open(%p)\n", file);


    id = file->f_inode->i_ino;
    current_node = head;

    while (current_node != NULL) {
        if (current_node->id == id) {
            current_node->open = 1;
            break;
        } else {
            current_node = current_node->next;
        }
    }

    if (current_node == NULL) {
        if (head == NULL) {
            head = kmalloc(sizeof(struct node), GFP_KERNEL);
            head->id = id;
            head->open = 1;
        } else {
            current_node = head;
            while (current_node->next != NULL) current_node = current_node->next;
            current_node->next = kmalloc(sizeof(struct node), GFP_KERNEL);
            current_node->next->id = id;
            current_node->next->open = 1;
        }
    }

    return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file) {
    int id;
    node_t* current_node;

    printk("device_release(%p,%p)\n", inode, file);

    id = file->f_inode->i_ino;

    current_node = head;

    while (current_node != NULL) {
        if (current_node->id == id) {
            current_node->open = 0;
            break;
        } else {
            current_node = current_node->next;
        }
    }

    return SUCCESS;
}


static ssize_t device_read(struct file *file, char __user * buffer, size_t length, loff_t * offset) {
    int i;
    int id;
    node_t* current_node;
    int index;

    printk("device_write(%p,%d)\n", file, length);

    id = file->f_inode->i_ino;

    current_node = head;

    while (current_node != NULL && current_node->id != id) current_node = current_node->next;

    if (current_node == NULL) {
        //  TODO file doesnt exist
    } else if (!(current_node->open)) {
        return -EINVAL;  //  TODO closed
    }

    index = current_node->data.index;
    //TODO check index

    for (i = 0; i < length; i++) {
        if (-EFAULT == put_user(current_node->data.buffers[index][i], buffer + i)) {
            //TODO handle error
        }
    }

    return i;
}


static ssize_t device_write(struct file *file, const char __user * buffer, size_t length, loff_t * offset) {
    int i;
    int id;
    node_t* current_node;
    int index;

    printk("device_write(%p,%d)\n", file, length);

    id = file->f_inode->i_ino;

    current_node = head;

    while (current_node != NULL && current_node->id != id) current_node = current_node->next;

    if (current_node == NULL) {
        //  TODO file doesnt exist
    } else if (!(current_node->open)) {
        //  TODO closed
    }

    index = current_node->data.index;
    //TODO check index

    for (i = 0; i < 128; i++) {
        if (i < length) {
            if (-EFAULT == get_user(current_node->data.buffers[index][i], buffer + i)) {
                //TODO handle error
            }
        } else {
            current_node->data.buffers[index][i] = '0';
        }
    }

    return i;
}

//----------------------------------------------------------------------------
static long device_ioctl(struct file* file, unsigned int ioctl_num, unsigned long ioctl_param) {
    printk("chardev, ioctl: setting index to %ld\n", ioctl_param);

    if (IOCTL_SET_ENC == ioctl_num && ioctl_param > -1 && ioctl_param < 4) {
        node_t* current_node;
        int id;

        printk("chardev, ioctl: setting index to %ld\n", ioctl_param);
        current_node = head;
        id = file->f_inode->i_ino;
        while (current_node->id != id) current_node = current_node->next;
        if (current_node->open) {
            current_node->data.index = ioctl_param;
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
    node_t* current_node;
    node_t* next;

    current_node = head;

    while (current_node != NULL) {
        next = current_node->next;
        kfree(current_node);
        current_node = next;
    }

    unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
}

module_init(simple_init);
module_exit(simple_cleanup);
