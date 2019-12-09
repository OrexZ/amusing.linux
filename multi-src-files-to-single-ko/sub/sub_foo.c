#include <linux/module.h>

void sub_print(void){
    printk(KERN_ERR "%s\n", __func__);
}
