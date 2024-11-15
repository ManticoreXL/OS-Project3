// #include <asm/syscall_wrapper.h>
#include <linux/highmem.h>
#include <linux/init.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/tracepoint.h>
#include <linux/types.h>

void **syscall_table;
void *(*original_os_ftrace)(struct pt_regs *);

static asmlinkage pid_t process_tracer(const struct pt_regs *regs)
{
    pid_t pid = (pid_t)regs->di;
    struct list_head *list;
    struct task_struct *task;
    struct task_struct *other;
    int count = 0;
    char pName[TASK_COMM_LEN];

    rcu_read_lock();
    task = pid_task(find_get_pid(pid), PIDTYPE_PID);

    // exception handle
    if (pid < 0)
        return -1;
    if (task == NULL)
        return -1;

    // print process' state info
    printk("##### TASK INFOMATION of ''[%d] %s'' #####\n", pid, task->comm);
    switch (task->state)
    {
    case TASK_RUNNING:
        printk(KERN_INFO "- task state : Running or ready\n");
        break;
    case TASK_INTERRUPTIBLE:
        printk(KERN_INFO "- task state : Wait\n");
        break;
    case TASK_UNINTERRUPTIBLE:
        printk(KERN_INFO "- task state : Wait with ignoring all signals\n");
        break;
    case TASK_STOPPED:
        printk(KERN_INFO "- task state : Stopped\n");
        break;
    case TASK_DEAD:
        printk(KERN_INFO "- task state : Dead\n");
        break;
    case EXIT_ZOMBIE:
        printk(KERN_INFO "- task state : Zombie process\n");
        break;
    default:
        printk(KERN_INFO "- task state : Undefined state\n");
        break;
    }

    // print process info
    strcpy(pName, task->group_leader->comm);
    if (task->group_leader != NULL)
        printk(KERN_INFO "- Process Group Leader : [%d] %s\n", task->group_leader->pid, pName);
    printk(KERN_INFO "- Number of context switches : %ld\n", task->nivcsw);
    printk(KERN_INFO "- Number of calling fork() : %ld\n", task->forkCount);
    printk(KERN_INFO "- it's parent process : [%d] %s\n", task->parent->pid, pName);

    // print sibiling processes's info
    printk(KERN_INFO "- it's sibiling process(es) :\n");
    list_for_each(list, &task->sibling)
    {
        other = list_entry(list, struct task_struct, sibling);
        if (other->pid > pid)
        {
            count++;
            strcpy(pName, other->comm);
            printk(KERN_INFO "  > [%d] %s\n", other->pid, pName);
        }
    }
    if (count == 0)
        printk(KERN_INFO "  > It has no sibling\n");
    else
        printk(KERN_INFO "  > This process has %d sibling process(es)\n", count);

    // print children processes' info
    count = 0;
    printk(KERN_INFO "- it's child process(es) :\n");
    list_for_each(list, &task->children)
    {
        other = list_entry(list, struct task_struct, sibling);
        count++;
        strcpy(pName, other->comm);
        printk(KERN_INFO "  > [%d] %s\n", other->pid, pName);
    }
    if (count == 0)
        printk(KERN_INFO "  > It has no child\n");
    else
        printk(KERN_INFO "  > This process has %d child process(es)\n", count);

    printk("##### END OF INFORMATION #####\n");

    return pid;
}

void make_rw(void *addr)
{
    // get write permission
    unsigned int level;
    pte_t *pte = lookup_address((u64)addr, &level);

    if (pte->pte & ~_PAGE_RW)
        pte->pte |= _PAGE_RW;
}

void make_ro(void *addr)
{
    // unset write permission
    unsigned int level;
    pte_t *pte = lookup_address((u64)addr, &level);

    pte->pte = pte->pte & ~_PAGE_RW;
}

static int __init tracer_init(void)
{
    syscall_table = (void **)kallsyms_lookup_name("sys_call_table");
    make_rw(syscall_table);

    original_os_ftrace = syscall_table[__NR_os_ftrace];
    syscall_table[__NR_os_ftrace] = process_tracer;

    return 0;
}

static void __exit tracer_exit(void)
{
    syscall_table[__NR_os_ftrace] = original_os_ftrace;
    make_ro(syscall_table);
}

module_init(tracer_init);
module_exit(tracer_exit);
MODULE_LICENSE("GPL");
