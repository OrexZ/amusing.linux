#include <linux/module.h>

void m_show(void){
    printk(KERN_ERR "%s\n", __func__);
}
