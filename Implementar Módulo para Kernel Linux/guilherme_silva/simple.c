#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/types.h>

struct birthday{
int day;
int month;
int year;
struct list_head list;
};

static LIST_HEAD(birthday_list);

int simple_init(void){      
       struct birthday *ptr;
       int i;
       for(i = 0; i < 5; i++){
              struct birthday *pessoa;
              pessoa = vmalloc(sizeof(*pessoa));
              pessoa->day= 06;
              pessoa->month= 10;
              pessoa->year = 1998;
              INIT_LIST_HEAD(&pessoa->list);
              list_add_tail(&pessoa->list, &birthday_list);
       }
       list_for_each_entry(ptr, &birthday_list, list){
              printk(KERN_INFO "Loading Module\n");
              printk(KERN_INFO "month:%d, day:%d, year:%d\n\n", ptr->month, ptr->day, ptr->year);
       }
       return 0;
}

void simple_exit(void){
    struct birthday *ptr, *next;
       list_for_each_entry_safe(ptr, next, &birthday_list, list) {
       printk(KERN_INFO "Removing Module\n");
       printk(KERN_INFO "month:%d, day:%d, year:%d\n\n", ptr->month, ptr->day, ptr->year);
       list_del(&ptr->list);
       vfree(ptr);
     }
}

module_init( simple_init );
module_exit( simple_exit );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Module");
MODULE_AUTHOR("SGG");
