#include <linux/init.h>
#include <linux/module.h>
#include "foo.h"

void __exit foo_exit(void){
    printk(KERN_ERR "sum: %d\n", m_sum(10, 1));
    printk(KERN_ERR "foo exit.\n");
}

module_exit(foo_exit);
