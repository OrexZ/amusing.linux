#include <linux/init.h>
#include <linux/module.h>
#include "foo.h"
#include "sub/sub_foo.h"

int __init foo_init(void){
    printk("%s\n", __func__);
    m_show();
    sub_print();
    return 0;
}

module_init(foo_init);

MODULE_LICENSE("GPL");

