/**
 * @file    s3019592Device.c
 * @author  Damon Toumbourou
 * @version 1.0
 * @date 2 August 2016
 * @brief   A very simple character driver. Maps to /dev/s3019592Device
 */

#include <linux/mutex.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#define DEVICE_NAME "s3019592Device"
#define CLASS_NAME "s3019592Class"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Damon Toumbourou");
MODULE_VERSION("1.0");

static int   majorNumber; // Stores the device number (auto determined)
static char  message[256]; // Memory for string passed from user space
static short size_of_message; // To remeber size of string being stored
static int   numOpen = 0; // Count number of times device opened
static struct class* s3019592Class = NULL;
static struct device* s3019592Device = NULL;

static DEFINE_MUTEX(s3019592Device_mutex);

// Prototype functions for the character driver 
static int  dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

/**
 *@brief Devices are represented as file structs in the kernel
 */
static struct file_operations fops = 
{
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
};

/**
 *@brief The LKM init function
 */
static int __init s3019592Device_init(void){
    printk(KERN_INFO "s3019592Device: Initialising the s3019592Device LKM\n");
    
    // dynamically allocate a major number for the device
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber < 0){
        printk(KERN_ALERT "s3019592Device failed to register a major number\n");
        
        return majorNumber;
    }
    
    printk(KERN_INFO "s3019592Device: registered correctly with major number %d\n", majorNumber);
    // Register the device class
    s3019592Class = class_create(THIS_MODULE, CLASS_NAME);
    
    if (IS_ERR(s3019592Class)){     // Check for errors and clean up if there is
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        
        return PTR_ERR(s3019592Class);
    }
    printk(KERN_INFO "s3019592Device: device class registered correctly\n");

    // Register the device driver
    s3019592Device = device_create(s3019592Class, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(s3019592Device)){ 
        class_destroy(s3019592Class);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        
        return PTR_ERR(s3019592Device);
    }
    // Init mutex
    mutex_init(&s3019592Device_mutex);

    // Device is init
    printk(KERN_INFO "s3019592Device: device class created correctly\n");
    return 0;
}

/**
 * @brief The LKM clean-up function 
 */
static void __exit s3019592Device_exit(void){
    mutex_destroy(&s3019592Device_mutex); // Destroy the mutex
    device_destroy(s3019592Class, MKDEV(majorNumber, 0));
    class_unregister(s3019592Class);
    class_destroy(s3019592Class);
    unregister_chrdev(majorNumber, DEVICE_NAME);
    printk(KERN_INFO "s3019592Device: exit\n");
}

/**
 * @brief The device open function. Called each tie the device is opened.
 * @param inodep A pointer to an inode object
 * @param filep A pointer to a file object
 */
static int dev_open(struct inode *inodep, struct file *filep){
    if(!mutex_trylock(&s3019592Device_mutex)){ // Try to aquire the mutex
        printk(KERN_ALERT "s3019592Device: Device in use by another process");
        return -EBUSY;
    }
    
    numOpen++;
    printk(KERN_INFO "s3019592Device: Device has been opened %d times\n", numOpen);
    return 0;
}

/**
 *@brief This function is called when the device is being read from user space.
 *       ie: data is being sent from the device to the user which would use the   *       copy_to_user() function
 */
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
    int error_count = 0;
    error_count = copy_to_user(buffer, message, size_of_message);

    if (error_count==0){
        printk(KERN_INFO "s3019592: Sent %d chars to the user\n", size_of_message);
        return 0;
    } else {
        printk(KERN_INFO "s3019592Device: failed to send %d chars to the user\n", error_count);
        return -EFAULT;
    }
}

/**
 *@brief This function is called when the device is being written to from the user space.
 */
static ssize_t dev_write(struct file *filep,const char *buffer, size_t len, loff_t *offset) {
    sprintf(message, "%s(%d letters)", buffer, len);
    size_of_message = strlen(message);
    printk(KERN_INFO "s3019592Device: Recieved %d chars from the user\n", len);
    return len;
}

/**
 * @brief This function releases the device 
 */
static int dev_release(struct inode *inodep, struct file *filep){
    mutex_unlock(&s3019592Device_mutex); // Release the mutex
    printk(KERN_INFO "s3019592Device: device successfully closed\n");
    return 0;
}

/**
 *
 */
module_init(s3019592Device_init);
module_exit(s3019592Device_exit);
