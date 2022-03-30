#include "my_info.h"
static int seq_show(struct seq_file *m, void *v)
{
    struct sysinfo s;
    struct cpuinfo_x86 *c;
    struct timespec uptime;
    struct timespec idle;
    int j,cpu{0};
    u64 nsec = 0;
    u32 rem;
    unsigned long pages[NR_LRU_LISTS];
    int lru;
    // unsigned long sreclaimable, sunreclaim;
    si_meminfo(&s);
    // si_swapinfo(&s);
    // committed = percpu_counter_read_positive(&vm_committed_as);
    // cached = global_node_page_state(NR_FILE_PAGES)-total_swapcache_pages()-s.bufferram;
    for (lru = LRU_BASE; lru < NR_LRU_LISTS; lru++)
        pages[lru] = global_node_page_state(NR_LRU_BASE + lru);

    //available = si_mem_available();
    //sreclaimable = global_node_page_state_pages(NR_SLAB_RECLAIMABLE);
    //sunreclaim = global_node_page_state_pages(NR_SLAB_UNRECLAIMABLE);
    //struct cpuinfo_x86 c = cpu_data(0);
    seq_puts(m, "\n=============Version=============\n");
    seq_printf(m, "Linux version %s\n", UTS_RELEASE);
    seq_puts(m, "\n=============CPU=============\n");
    for_each_possible_cpu(cpu)
    {
        c = &cpu_data(cpu);
        seq_printf(m, "processor\t: %u\n", c->cpu_index);
        seq_printf(m, "model name\t: %s\n", c->x86_model_id);
        seq_printf(m, "physical id\t: %u\n", c->phys_proc_id);
        seq_printf(m, "core id\t\t: %u\n", c->cpu_core_id);
        seq_printf(m, "cpu cores\t: %u\n", c->booted_cores);
        seq_printf(m, "cache size\t: %u KB\n", c->x86_cache_size);
        seq_printf(m, "clflush size\t: %u\n", c->x86_clflush_size);
        seq_printf(m, "cache_alignment\t: %u\n", c->x86_cache_alignment);
        seq_printf(m, "address sizes\t: %u bits physical, %u bits virtual\n\n", c->x86_phys_bits, c->x86_virt_bits);
    }
    seq_puts(m, "\n=============Memory============\n");
    seq_printf(m, "MemTotal\t: %lu kB\n", (s.totalram)<<(PAGE_SHIFT-10));
    seq_printf(m, "MemFree\t\t: %lu kB\n", (s.freeram)<<(PAGE_SHIFT-10));
    seq_printf(m, "Buffers\t\t: %lu kB\n", (s.bufferram)<<(PAGE_SHIFT-10));
    seq_printf(m, "Active\t\t: %lu kB\n", (pages[LRU_ACTIVE_ANON] + pages[LRU_ACTIVE_FILE])<<(PAGE_SHIFT-10));
    seq_printf(m, "Inactive\t: %lu kB\n", (pages[LRU_INACTIVE_ANON] + pages[LRU_INACTIVE_FILE])<<(PAGE_SHIFT-10));
    seq_printf(m, "Shmem\t\t: %lu kB\n", (s.sharedram)<<(PAGE_SHIFT-10));
    seq_printf(m, "Dirty\t\t: %lu kB\n", (global_node_page_state(NR_FILE_DIRTY))<<(PAGE_SHIFT-10));
    seq_printf(m, "Writeback\t: %lu kB\n", (global_zone_page_state(NR_WRITEBACK))<<(PAGE_SHIFT-10));
    seq_printf(m, "KernelStack\t: %lu kB\n", global_zone_page_state(NR_KERNEL_STACK_KB));
    seq_printf(m, "PageTables\t: %lu kB\n", (global_zone_page_state(NR_PAGETABLE))<<(PAGE_SHIFT-10));
    seq_puts(m, "\n=============Time============\n");

    for_each_possible_cpu(j)
    nsec += (__force u64)kcpustat_cpu(j).cpustat[CPUTIME_IDLE];
    get_monotonic_boottime(&uptime);
    idle.tv_sec = div_u64_rem(nsec, NSEC_PER_SEC, &rem);
    idle.tv_nsec = rem;
    seq_printf(m, "Uptime\t\t: %lu.%02lu (s)\nIdletime\t: %lu.%02lu (s)\n\n",
               (unsigned long)uptime.tv_sec,
               (uptime.tv_nsec / (NSEC_PER_SEC / 100)),
               (unsigned long)idle.tv_sec,
               (idle.tv_nsec / (NSEC_PER_SEC / 100)));
    return 0;
}
/*
static const struct seq_operations seq_ops = {
    .start = seq_start,
    .next = seq_next,
    .stop = seq_stop,
    .show = seq_show};
*/
static int info_open(struct inode *inode, struct file *file)
{
    return single_open(file, seq_show, NULL);
}
static const struct file_operations my_info_proc_fops =
{
    .owner = THIS_MODULE,
    .open = info_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release
};

static int __init info_init(void)
{
    struct proc_dir_entry *entry;
    entry = proc_create("my_info", 0, NULL, &my_info_proc_fops);
    if (entry == NULL)
    {
        printk(KERN_INFO "entry = NULL\n");
        return -ENOMEM;
    }
    else
    {
        printk(KERN_INFO "entry =/= NULL");
    }
    return 0;
}
static void __exit info_exit(void)
{
    printk(KERN_INFO "exit\n");
    remove_proc_entry("my_info", NULL);
}

MODULE_LICENSE("GPL");
module_init(info_init);
module_exit(info_exit);
