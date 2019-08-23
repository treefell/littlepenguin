#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

MODULE_LICENSE("Dual MIT/GPL");
MODULE_DESCRIPTION("misc module");
#define BUF_LEN 6
#define LOGIN "chuang"

static int module_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "i'm online!");
	return 0;
}

static int module_close(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "i'm offline!");
	return 0;
}

static ssize_t module_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
	int ret;
	char w_buf[BUF_LEN];

	if (len  != BUF_LEN){
		printk(KERN_INFO "len %zu buf len %zu\n",len,(size_t)BUF_LEN);
		goto fail;
	}
	ret = copy_from_user(w_buf, buf, BUF_LEN);
	printk(KERN_INFO "ret %d\n", ret);
	if (strncmp(buf, LOGIN, BUF_LEN) == 0){
		printk(KERN_INFO "correct value\n");
		return BUF_LEN;
	}
fail:
	printk(KERN_INFO "invalide value\n");
	return -EINVAL;
}

static ssize_t module_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
	//printk(KERN_INFO "chuang\n");
	int	ret;
	char	*read_pos = LOGIN + *ppos;
	size_t	read_len = len > (BUF_LEN - *ppos) ? (BUF_LEN - *ppos): len;

	if (read_len == 0)
		return 0;
	ret = copy_to_user(buf, read_pos, read_len);
	if (ret == read_len)
		return 0;
	else{
		*ppos = BUF_LEN - ret;
		ret = read_len - ret;
		return ret;
	}
	return -EINVAL;
}

static const struct file_operations module_fops = {
	.write = module_write,
	.read = module_read,
	.open = module_open,
	.release = module_close,
};

struct miscdevice fortytwo_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "fourtytwo",
	.fops = &module_fops,
};


int init_module(void)
{
	if(misc_register(&fortytwo_device) != 0)
		printk(KERN_INFO "Hello misc!\n");
	return 0;
}
void cleanup_module(void)
{
	printk(KERN_INFO "Goodbye misc.\n");
	misc_deregister(&fortytwo_device);
}
