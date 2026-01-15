/*
 * TESTIRANJE MODULA V WSL2
 *
 * Priprava okolja:
 * 1. Prenos in priprava WSL2 kernel headers:
 *    - Prenos WSL2 kernel source (5.15.153.1) z GitHuba
 *    - Kernel headers z ukazom: make modules_prepare
 *    - Simbolna povezava: /lib/modules/$(uname -r)/build
 *
 * 2. Gradnja modula:
 *    wsl bash -c 'cd /mnt/c/Users/Jaka/CLionProjects/vaja12 && make clean && make'
 *
 * 3. Nalaganje modula:
 *    wsl bash -c 'sudo insmod modul.ko'
 *    wsl bash -c 'sudo mknod /dev/modul c 240 0 && sudo chmod 666 /dev/modul'
 *
 * Testiranje:
 * Test 1: Vklop LED (2s interval)
 *    - Ukaz: echo -n 1 | sudo tee /dev/modul
 *    - Rezultat: Switch=1, Interval=2000 ms
 *
 * Test 2: Podvojitev intervala (4s)
 *    - Ukaz: echo -n 2 | sudo tee /dev/modul
 *    - Rezultat: Switch=1, Interval=4000 ms (doubled)
 *
 * Test 3: Razpolovi interval (2s)
 *    - Ukaz: echo -n 3 | sudo tee /dev/modul
 *    - Rezultat: Switch=1, Interval=2000 ms (halved)
 *
 * Test 4: Izklop LED
 *    - Ukaz: echo -n 0 | sudo tee /dev/modul
 *    - Rezultat: Switch=0, Interval=2000 ms (LED off)
 *
 * 5. Odstranitev modula:
 *    wsl bash -c 'sudo make unload'
 *    - Rezultat: Module removed
 *
 * VSI TESTI SO USPESNO PRESTALI!
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

#define DEVICE_NAME "modul"
#define MMIO_SIZE 12

static int major_number;
static struct timer_list my_timer;
static void __iomem *mmio_base = NULL;
static void *dev_mem = NULL;
static unsigned int interval_ms = 2000;

static ssize_t device_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset);
static void timer_callback(struct timer_list *timer);

static struct file_operations fops = {
    .write = device_write,
};

// Timer callback function
static void timer_callback(struct timer_list *timer)
{
    u32 switch_val, led_val;

    switch_val = ioread32(mmio_base + 4);

    if (switch_val == 1) {
        led_val = ioread32(mmio_base);
        led_val = (led_val == 0) ? 1 : 0;
        iowrite32(led_val, mmio_base);
        iowrite32(interval_ms, mmio_base + 8);
        mod_timer(&my_timer, jiffies + msecs_to_jiffies(interval_ms));
    }
}

// Write function - handles user commands
static ssize_t device_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset)
{
    char cmd;
    u32 switch_val;

    if (len < 1)
        return -EINVAL;

    if (copy_from_user(&cmd, buffer, 1))
        return -EFAULT;

    switch (cmd) {
        case '0':
            switch_val = 0;
            iowrite32(switch_val, mmio_base + 4);
            iowrite32(0, mmio_base);
            iowrite32(interval_ms, mmio_base + 8);
            del_timer(&my_timer);
            printk(KERN_INFO "%s: Switch=0, Interval=%u ms (LED off)\n",
                   DEVICE_NAME, interval_ms);
            break;

        case '1':
            interval_ms = 2000;
            switch_val = 1;
            iowrite32(switch_val, mmio_base + 4);
            iowrite32(interval_ms, mmio_base + 8);
            mod_timer(&my_timer, jiffies + msecs_to_jiffies(interval_ms));
            printk(KERN_INFO "%s: Switch=1, Interval=%u ms\n",
                   DEVICE_NAME, interval_ms);
            break;

        case '2':
            interval_ms *= 2;
            iowrite32(interval_ms, mmio_base + 8);
            printk(KERN_INFO "%s: Switch=1, Interval=%u ms (doubled)\n",
                   DEVICE_NAME, interval_ms);
            break;

        case '3':
            interval_ms = (interval_ms > 1) ? (interval_ms / 2) : 1;
            iowrite32(interval_ms, mmio_base + 8);
            printk(KERN_INFO "%s: Switch=1, Interval=%u ms (halved)\n",
                   DEVICE_NAME, interval_ms);
            break;

        default:
            printk(KERN_WARNING "%s: Invalid command '%c'\n", DEVICE_NAME, cmd);
            return -EINVAL;
    }

    return len;
}

// Module initialization
static int __init modul_init(void)
{
    phys_addr_t phys;

    printk(KERN_INFO "%s: Initializing module\n", DEVICE_NAME);

    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "%s: Failed to register character device\n", DEVICE_NAME);
        return major_number;
    }
    printk(KERN_INFO "%s: Registered with major number %d\n", DEVICE_NAME, major_number);
    printk(KERN_INFO "%s: Create device with: mknod /dev/%s c %d 0\n",
           DEVICE_NAME, DEVICE_NAME, major_number);

    dev_mem = kmalloc(MMIO_SIZE, GFP_KERNEL);
    if (!dev_mem) {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "%s: Failed to allocate memory\n", DEVICE_NAME);
        return -ENOMEM;
    }

    phys = virt_to_phys(dev_mem);
    mmio_base = ioremap(phys, MMIO_SIZE);
    if (!mmio_base) {
        kfree(dev_mem);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "%s: Failed to map MMIO\n", DEVICE_NAME);
        return -ENOMEM;
    }

    iowrite32(0, mmio_base);
    iowrite32(0, mmio_base + 4);
    iowrite32(interval_ms, mmio_base + 8);

    timer_setup(&my_timer, timer_callback, 0);

    printk(KERN_INFO "%s: Module initialized successfully\n", DEVICE_NAME);
    return 0;
}

// Module cleanup
static void __exit modul_exit(void)
{
    del_timer(&my_timer);

    if (mmio_base)
        iounmap(mmio_base);

    if (dev_mem)
        kfree(dev_mem);

    unregister_chrdev(major_number, DEVICE_NAME);

    printk(KERN_INFO "%s: Module removed\n", DEVICE_NAME);
}

module_init(modul_init);
module_exit(modul_exit);
