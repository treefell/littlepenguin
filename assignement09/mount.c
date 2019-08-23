#include <linux/module.h>
#include <linux/dcache.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <../fs/mount.h>
#include <linux/list.h>
#include <linux/namei.h>
#include <linux/seq_file.h>
#include <linux/nsproxy.h>

#define PROCFS_NAME "mymounts"

MODULE_LICENSE("Dual MIT/GPL");
MODULE_DESCRIPTION("mymounts");
static struct proc_dir_entry *mount_file;

static void *seq_start(struct seq_file *file, loff_t *pos)
{
	struct mount *mymounts;

	if (*pos >= current->nsproxy->mnt_ns->mounts)
		return NULL;
	mymounts = list_first_entry(&current->nsproxy->mnt_ns->list,
				struct mount, mnt_list);
	return mymounts;

}

static void *seq_next(struct seq_file *file, void *data, loff_t *pos)
{
	struct mount *mymount;
	
	(*pos)++;
	if (*pos >= current->nsproxy->mnt_ns->mounts) {
		mymount = NULL;
		goto end;
	}
	mymount = (struct mount *)data;
	mymount = list_next_entry(mymount, mnt_list);
end:
	return mymount;
}

static void seq_stop(struct seq_file *file, void *data)
{
	(void)file;
	(void)data;
}

static int seq_show(struct seq_file *file, void *data)
{
	struct mount *mymount;
	char string[1024];
	struct path path;
	char *pathstring;

	mymount = (struct mount *)data;
	path.mnt = &mymount->mnt_parent->mnt;
	path.dentry = mymount->mnt_mountpoint;
	pathstring = d_path(&path, string, 1024);
	seq_printf(file, "%-20s\t%s\n", mymount->mnt_devname, pathstring);

	return 0;
}


static const struct seq_operations mount_seq_ops = {
	.start = seq_start,
	.next = seq_next,
	.show = seq_show,
	.stop = seq_stop,
};

static int mount_proc_open(struct inode *inodep, struct file *file)
{
	(void)inodep;
	return seq_open(file, &mount_seq_ops);
}

static const struct file_operations mount_fops = {
	.owner = THIS_MODULE,
	.open = mount_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

int init_module()
{
	mount_file = proc_create(PROCFS_NAME, 0, NULL, &mount_fops);
	return(0);
}

void cleanup_module()
{
	proc_remove(mount_file);
}
