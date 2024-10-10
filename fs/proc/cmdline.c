// SPDX-License-Identifier: GPL-2.0
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#ifdef CONFIG_BOOTLOADER_ARGUMENTS_SPOOF
#include <asm/setup.h>
static char new_command_line[COMMAND_LINE_SIZE];
#endif

static int cmdline_proc_show(struct seq_file *m, void *v)
{
#ifdef CONFIG_BOOTLOADER_ARGUMENTS_SPOOF
        seq_printf(m, "%s\n", new_command_line);
#else
        seq_printf(m, "%s\n", saved_command_line);
#endif
	return 0;
}

static int cmdline_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, cmdline_proc_show, NULL);
}

#ifdef CONFIG_BOOTLOADER_ARGUMENTS_SPOOF
static void patch_flag(char *cmd, const char *flag, const char *val)
{
	size_t flag_len, val_len;
	char *start, *end;

	start = strstr(cmd, flag);
	if (!start)
		return;

	flag_len = strlen(flag);
	val_len = strlen(val);
	end = start + flag_len + strcspn(start + flag_len, " ");
	memmove(start + flag_len + val_len, end, strlen(end) + 1);
	memcpy(start + flag_len, val, val_len);
}

static void patch_bootloader_arguments(char *cmd)
{
	patch_flag(cmd, "androidboot.verifiedbootstate=", "green");
	patch_flag(cmd, "androidboot.veritymode=", "enforcing");
	patch_flag(cmd, "androidboot.vbmeta.device_state=", "locked");
	patch_flag(cmd, "androidboot.warranty_bit=", "0");
	
#ifdef CONFIG_D1Q_SPOOF
        patch_flag(cmd, "androidboot.em.model=", "SM-S911B ");
	patch_flag(cmd, "androidboot.bootloader=", "S911BXXS6CXHA");
	patch_flag(cmd, "androidboot.product.model=", "SM-S911B ");
#endif
}
#endif

static const struct file_operations cmdline_proc_fops = {
	.open		= cmdline_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init proc_cmdline_init(void)
{
	proc_create("cmdline", 0, NULL, &cmdline_proc_fops);
#ifdef CONFIG_BOOTLOADER_ARGUMENTS_SPOOF
        strcpy(new_command_line, saved_command_line);
        patch_bootloader_arguments(new_command_line);
#endif
	return 0;
}
fs_initcall(proc_cmdline_init);
