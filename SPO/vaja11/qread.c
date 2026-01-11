#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/cdev.h>

#define DEVICE_NAME "qread"

// Parametri modula
static int MEM_SIZE = 128;
module_param(MEM_SIZE, int, S_IRUGO);
static int BLK_SIZE = 32;
module_param(BLK_SIZE, int, S_IRUGO);

static char *ptr = NULL;
static dev_t dev_num;
static struct cdev q_cdev;
static struct class *q_class;

// Funkcija za branje (kvantizirano po BLK_SIZE)
static ssize_t q_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
    ssize_t to_copy;
    
    // Če smo na koncu pomnilnika, vrnemo 0
    if (*off >= MEM_SIZE) return 0;

    // Kvantizacija: beremo največ BLK_SIZE bajtov naenkrat
    to_copy = (len < BLK_SIZE) ? len : BLK_SIZE;

    // Preverimo, da ne beremo čez MEM_SIZE
    if (*off + to_copy > MEM_SIZE) {
        to_copy = MEM_SIZE - *off;
    }

    if (copy_to_user(buf, ptr + *off, to_copy)) {
        return -EFAULT;
    }

    *off += to_copy;
    return to_copy;
}

// Funkcija za pisanje
static ssize_t q_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
    size_t actual_to_write;

    if (*off >= MEM_SIZE) return len; // Vrnemo len, da "zadovoljimo" aplikacijo

    actual_to_write = len;
    if (*off + len > MEM_SIZE) {
        actual_to_write = MEM_SIZE - *off;
    }

    if (copy_from_user(ptr + *off, buf, actual_to_write)) {
        return -EFAULT;
    }

    *off += actual_to_write;
    return len; // Vedno vrnemo prvotno len po navodilih
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = q_read,
    .write = q_write,
};

static int __init qread_init(void) {
    // Rezervacija pomnilnika
    ptr = kmalloc(MEM_SIZE, GFP_KERNEL);
    if (!ptr) return -ENOMEM;
    memset(ptr, 0, MEM_SIZE);

    // Registracija naprave
    alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    cdev_init(&q_cdev, &fops);
    cdev_add(&q_cdev, dev_num, 1);
    
    q_class = class_create("qread_class");
    device_create(q_class, NULL, dev_num, NULL, DEVICE_NAME);

    printk(KERN_INFO "qread: Naložen (MEM_SIZE=%d, BLK_SIZE=%d)\n", MEM_SIZE, BLK_SIZE);
    return 0;
}

static void __exit qread_exit(void) {
    device_destroy(q_class, dev_num);
    class_destroy(q_class);
    cdev_del(&q_cdev);
    unregister_chrdev_region(dev_num, 1);
    kfree(ptr);
    printk(KERN_INFO "qread: Odstranjen\n");
}

module_init(qread_init);
module_exit(qread_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Student");
