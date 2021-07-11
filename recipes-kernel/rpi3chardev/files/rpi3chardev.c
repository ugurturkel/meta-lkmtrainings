#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h> 

MODULE_LICENSE("GPL");            
MODULE_AUTHOR("Ugur Turkel");    
MODULE_DESCRIPTION("Basic Char Device Driver");  
MODULE_VERSION("0.1");            

static struct class* rpi3chardevClass  = NULL;
static struct device* rpi3chardevDevice = NULL;

static char usrMsg[1024] = {0};
static int usrMsgSize = 0;
static int majorNumber = 0;
static int openedCounter = 0;

static int cdev_open(struct inode* inodeptr, struct file* fileptr);
static int cdev_release(struct inode* inodeptr, struct file* fileptr);
static ssize_t cdev_read(struct file* fileptr, char __user* buffer, size_t msgLength, loff_t* offset);
static ssize_t cdev_write(struct file* fileptr, const char __user* buffer, size_t msgLength, loff_t* offset);

static struct file_operations fops =
{
	.open = cdev_open,
	.read =  cdev_read,
	.write = cdev_write,
	.release = cdev_release,
};

static int __init rpi3chardev_init(void)
{
	printk(KERN_INFO "rpi3chardev: Initializing...\n");

	majorNumber = register_chrdev(0, "rpi3chardev", &fops);
	if (majorNumber < 0)
	{
		printk(KERN_ALERT "rpi3chardev: registering with a major number is failed.\n");
		return majorNumber;
	}

	printk(KERN_INFO "rpi3chardev: registered with major number: %d\n", majorNumber);


	rpi3chardevClass = class_create(THIS_MODULE, "brpi3"); //create the class.

	if (IS_ERR(rpi3chardevClass) == 1)
	{
      unregister_chrdev(majorNumber, "rpi3chardev");
      printk(KERN_ALERT "registering device class is failed.\n");
      return PTR_ERR(rpi3chardevClass);
	}

	printk(KERN_INFO "rpi3chardev: registered the device.\n");

   
	rpi3chardevDevice = device_create(rpi3chardevClass, NULL, MKDEV(majorNumber, 0), NULL, "rpi3chardev");
	if (IS_ERR(rpi3chardevDevice) == 1)
	{
		class_destroy(rpi3chardevClass);
		unregister_chrdev(majorNumber, "rpi3chardev");
		printk(KERN_ALERT "creating the device is failed.\n");
		return PTR_ERR(rpi3chardevDevice);
	}

	printk(KERN_INFO "rpi3chardev: device class is created.\n");
	return 0;
}


static void __exit rpi3chardev_exit(void)
{
	device_destroy(rpi3chardevClass, MKDEV(majorNumber, 0));
	class_unregister(rpi3chardevClass);                          
	class_destroy(rpi3chardevClass);                             
	unregister_chrdev(majorNumber, "rpi3chardev");             
	printk(KERN_INFO "rpi3chardev: device exit.\n");
}


static ssize_t cdev_read(struct file* fileptr, char __user* buffer, size_t msgLength, loff_t* offset)
{
	int errorCounter = 0;
	errorCounter = copy_to_user(buffer, usrMsg, usrMsgSize);

	if (errorCounter == 0)
	{
      printk(KERN_INFO "rpi3chardev: Sent %d characters to the user\n", usrMsgSize);
      return (usrMsgSize = 0);
	}
	else 
	{
      printk(KERN_INFO "rpi3chardev: Failed to send %d characters.\n", errorCounter);
      return -EFAULT;
	}
}

static ssize_t cdev_write(struct file* fileptr, const char __user* buffer, size_t msgLength, loff_t* offset)
{
	sprintf(usrMsg, "%s(%zu letters)", buffer, msgLength);
	usrMsgSize = strlen(usrMsg);
	printk(KERN_INFO "rpi3chardev: received %zu characters.\n", msgLength);
	return msgLength;
}

static int cdev_release(struct inode *inodeptr, struct file* fileptr)
{
	printk(KERN_INFO "rpi3chardev: closed the device.\n");
	return 0;
}

static int cdev_open(struct inode* inodeptr, struct file* fileptr)
{
	openedCounter++;
	printk(KERN_INFO "rpi3chardev: device opened %d times.\n", openedCounter);
	return 0;
}

module_init(rpi3chardev_init);
module_exit(rpi3chardev_exit);
