#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include <linux/debugfs.h>

MODULE_LICENSE("Dual MIT/GPL");
MODULE_DESCRIPTION("debugfs_dir module");
#define BUF_LEN 6
#define LOGIN "chuang"

static struct	dentry *debugfs_dir;
static char	foo_data[PAGE_SIZE];
static size_t	foo_len;
static struct mutex mutex_foo;

static ssize_t id_module_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
	int ret;
	char w_buf[BUF_LEN];

	if (len != BUF_LEN){
		ret = -EINVAL;
		goto fail;
	}
	ret = copy_from_user(w_buf, buf, BUF_LEN);
	if (strncmp(buf, LOGIN, BUF_LEN) == 0){
		ret = BUF_LEN;
	}
fail:
	return ret;
}

static ssize_t id_module_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
	int	ret;
	char	*read_pos = LOGIN + *ppos;
	size_t	read_len = len > (BUF_LEN - *ppos) ? (BUF_LEN - *ppos): len;

	if (read_len == 0){
		ret = 0;
		goto out;
	}
	ret = copy_to_user(buf, read_pos, read_len);
	if (ret == read_len){
		ret = 0;
		goto out;
	}
	else{
		*ppos = BUF_LEN - ret;
		ret = read_len - ret;
		goto out;
	}
out:
	return ret;
}

static const struct file_operations id_fops = {
	.write = id_module_write,
	.read = id_module_read,
};

static ssize_t foo_module_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
	int ret;

	if (*ppos >= PAGE_SIZE){
		ret = -E2BIG;
		goto out;
	}
	ret = mutex_lock_interruptible(&mutex_foo);
	if (ret)
		goto skip;
	if (*ppos + len >= PAGE_SIZE){
		len = (size_t)PAGE_SIZE - *ppos;
	}
	ret = copy_from_user(foo_data + *ppos, buf, len);
	if(ret == len){
		ret = -EINVAL;
		goto out;
	}
	ret = len - ret;
	*ppos += ret;
	foo_len = *ppos;
	printk(KERN_INFO "done");
out:
	mutex_unlock(&mutex_foo);
skip:
	return ret;
}

static ssize_t foo_module_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
	int	ret = 0;
	char	*read_pos = foo_data + *ppos;
	size_t	read_len = len > (foo_len - *ppos) ? (foo_len - *ppos): len;
	
	ret = mutex_lock_interruptible(&mutex_foo);
	if (read_len == 0)
		goto out;
	ret = copy_to_user(buf, read_pos, read_len);
	if (ret == read_len){
		ret = 0;
		goto out;
	}
	else{
		*ppos = foo_len - ret;
		ret = read_len - ret;
	}
	out:
		mutex_unlock(&mutex_foo);
		return ret;
}

static const struct file_operations foo_fops = { 
	.write = foo_module_write,
	.read = foo_module_read,
};

static struct dentry *create_jiffies(void)
{	
	return (BITS_PER_LONG == 32 ? 
	debugfs_create_u32("jiffies", 0444, debugfs_dir, (u32*)&jiffies) 
	: debugfs_create_u64("jiffies", 0444, debugfs_dir, (u64*)&jiffies));
}

int init_module(void)
{
	int ret = 0;

	printk(KERN_INFO "Hello debugfs!\n");
	debugfs_dir = debugfs_create_dir("fortytwo",NULL);
	if (!debugfs_dir)
	{
		ret = -ENODEV;
		goto exit;
	}
	if (!create_jiffies()){
		ret = -ENOENT;
	}
	if (!debugfs_create_file("id", 0666, debugfs_dir, NULL, &id_fops)){
		ret = -ENOENT;
		goto exit;
	}
	if (!debugfs_create_file("foo", 0644, debugfs_dir, NULL, &foo_fops))
		ret = -ENOENT;
exit:
	return ret;
}

void cleanup_module(void)
{
	printk(KERN_INFO "Goodbye debugfs.\n");
	debugfs_remove_recursive(debugfs_dir);
}
