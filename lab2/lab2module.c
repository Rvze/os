#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/pid.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/dcache.h>
#include <linux/net.h>
#include <linux/fdtable.h>
#include <net/sock.h>
#include <linux/netdevice.h>
#include <linux/device.h>


#define BUFFER_SIZE 1024

MODULE_LICENSE("Dual BSD/GPL");

static struct dentry *debug_dir;
static struct dentry *debug_file;
static struct task_struct* task = NULL;
static void print_address_space(struct seq_file *file, struct task_struct *task);
static void print_net_device(struct seq_file *file);
static int print_struct(struct seq_file *file, void *data);

static ssize_t write_function(struct file *file, const char __user *buffer, size_t length, loff_t *ptr_offset) {
  char user_data[BUFFER_SIZE];
  unsigned long pid;
  copy_from_user(user_data, buffer, length);
  sscanf(user_data, "pid: %ld", &pid);
  task = get_pid_task(find_get_pid(pid),PIDTYPE_PID);
  single_open(file, print_struct, NULL);
  return strlen(user_data);
}

static struct file_operations fops = {
  .read = seq_read,
  .write = write_function,
};


static int __init mod_init(void) {
  debug_dir = debugfs_create_dir("lab2", NULL);
  debug_file = debugfs_create_file("filetocnange", 0644, debug_dir, NULL, &fops);
  return 0;
}

static void __exit mod_exit(void) {
  debugfs_remove_recursive(debug_dir);
}


static int print_struct(struct seq_file *file, void *data) {
  print_address_space(file, task);
  print_net_device(file);
  return 0;
}

static void print_net_device(struct seq_file *file) {
  static struct net_device *net_device;
  read_lock(&dev_base_lock);
  net_device = first_net_device(&init_net);
  seq_printf(file, "net devices : {\n");
  while(net_device){
    seq_printf(file,"\tname = %s\n",net_device->name);
    seq_printf(file,"\tmem_start = %lu\n",net_device->mem_start);
    seq_printf(file,"\tmem_end = %lu\n",net_device->mem_end);
    seq_printf(file,"\tbase_addr = %lu\n",net_device->base_addr);
    seq_printf(file,"\tmtu = %lu\n",net_device->mtu);
    seq_printf(file,"\tmin_mtu = %lu\n",net_device->min_mtu);
    seq_printf(file,"\tmax_mtu = %lu\n",net_device->max_mtu);
    seq_printf(file,"\tflags = %lu\n\n",net_device->flags);
    net_device = next_net_device(net_device);
  }
  read_unlock(&dev_base_lock);
  seq_printf(file, "}\n");	  
}

static void print_address_space(struct seq_file *file, struct task_struct *task) {
  static struct address_space *address_space_s;
  if (task == NULL) {
    seq_printf(file, "Can't find address_space\n");
  } else {
    if (task->mm==0){
       seq_printf(file, "Can't find address_space\n");
    } else {
      address_space_s = task->mm->mmap->vm_file->f_path.dentry->d_inode->i_mapping;
      seq_printf(file, "address_space : {\n");
      seq_printf(file,"\thost_number = %lu\n",address_space_s->host->i_ino);
      seq_printf(file,"\tnrpages = %lu\n",address_space_s->nrpages);
      seq_printf(file,"\tflags = %lu\n",address_space_s->flags);
      seq_printf(file, "}\n");
    }
     
  }
  
}


module_init(mod_init);
module_exit(mod_exit);
