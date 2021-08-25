#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/gpio.h> //the gpio library for using raspberry pi gpio pins

MODULE_LICENSE("GPL");            
MODULE_AUTHOR("Ugur Turkel");    
MODULE_DESCRIPTION("Adventure of a button and a led");
MODULE_VERSION("0.1");            

static uint8_t ledpin = 23;
static uint8_t buttonpin = 24;
static uint8_t irqnum;

static irq_handler_t irq_handler_func(uint16_t irq, void *dev_id, struct pt_regs *regs); //custom interrupt function's prototype in irq_handler_t type
//dev_id is for shared interrupt line, in our case it will be NULL
//pt_regs structure is for storing the system calls, a system call is a programmatic way a program requests a service from the kernel 
static int __init buttonandled_init(void)
{
	uint8_t reqresult = 0;
	printk(KERN_INFO "Button and Led Driver: loading the kernel module...\n");
	if(gpio_is_valid(ledpin) == 0){
		printk(KERN_ERR "gpio pin is invalid.");
		return -ENODEV;
	}
	gpio_request(ledpin, "sysfs"); //request the gpio pin from syfs
	gpio_direction_output(ledpin, 1);
	gpio_set_value(ledpin, 1);
	gpio_export(ledpin, 0); //exporting creates a gpio file access
	gpio_request(buttonpin, "sysfs");
	gpio_direction_input(buttonpin); //make button pin input
	gpio_set_debounce(buttonpin, 50);
	gpio_export(buttonpin, 0);
	irqnum = gpio_to_irq(buttonpin); //detect the related irq for that specific pin number
	printk(KERN_INFO "IRQ number is: %d\n", irqnum);
	printk(KERN_INFO "Button and Led Driver: current button state: %d\n", gpio_get_value(buttonpin));
	printk(KERN_INFO "Button and Led Driver: current led state: %d\n", gpio_get_value(ledpin));
	reqresult = request_irq(irqnum, (irq_handler_t)irq_handler_func, IRQF_TRIGGER_RISING, "button_and_led_handler", NULL); //it can be triggered on FALLING or RISING, 
	printk(KERN_INFO "Button and Led Driver: IRQ request result is: %d\n", reqresult);
	return reqresult;
}


static void __exit buttonandled_exit(void)
{
	printk(KERN_INFO "Button and Led Driver: current button state: %d\n",gpio_get_value(buttonpin));
	printk(KERN_INFO "Button and Led Driver: current led state: %d\n", gpio_get_value(ledpin));
	gpio_set_value(ledpin, 0);
	gpio_unexport(ledpin);
	free_irq(irqnum, NULL);
	gpio_unexport(buttonpin);
	gpio_free(ledpin);
	gpio_free(buttonpin);
	printk(KERN_INFO "Button and Led Driver: unloading the kernel module...\n");
}

static irq_handler_t irq_handler_func(uint16_t irq, void *dev_id, struct pt_regs *regs)
{
	if(gpio_get_value(ledpin) == 1) gpio_set_value(ledpin, 0);
	else if(gpio_get_value(ledpin) == 0) gpio_set_value(ledpin, 1);
	printk(KERN_INFO "Button and Led Driver: button is pressed.");
	printk(KERN_INFO "led state inverted.");
	return (irq_handler_t)IRQ_HANDLED;
}

module_init(buttonandled_init);
module_exit(buttonandled_exit);
