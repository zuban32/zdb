#include "break.h"
#include "elf.h"

char *cut_func_name(char *source)
{
    char *src = source;
    unsigned len = 0;
    while (*src != ':' && *src != 0)
    {
        len++, src++;
    }
    src = calloc(len + 1, sizeof(*src));
    memmove(src, source, len);
    return src;
}

void add_breakpoint(Elf32_file *file, char *name, unsigned addr, int orig_info)
{
    Breakpoint *cur = get_tail(file);
    if (!cur)
    {
        file->breaks = calloc(1, sizeof(*(file->breaks)));
        cur = file->breaks;
    }
    else
    {
        cur->next = calloc(1, sizeof(*cur));
        cur = cur->next;
    }
    cur->addr = addr;
    cur->next = NULL;
    cur->func_name = calloc(strlen(name) + 1, sizeof(*(cur->func_name)));
    memmove(cur->func_name, name, strlen(name) + 1);
    cur->orig_info = orig_info;
    cur->status = 1;
}


int set_breakpoint(Elf32_file *file, pid_t pid, char *name, const unsigned numline)
{
    Breakpoint *cur = file->breaks;
    while (cur)
    {
        if (!strcmp(cur->func_name, name))
        {
            printf("(zdb)\tBreakpoint already exists\n");
            return 0;
        }
        cur = cur->next;
    }
    int i = 1;
    //unsigned cur_start_addr = 0;
    while (i < file->stabs_num)
    {
        /*if(stabs[i].n_type == N_SO)
            cur_start_addr = stabs[i].n_value;*/
        if (file->stabs[i].n_type == N_FUN)
        {
            char *func_name = cut_func_name(file->stabstr + file->stabs[i].n_strx);
            if (!strcmp(name, func_name))
            {
                int orig_info = ptrace(PTRACE_PEEKTEXT, pid, file->stabs[i].n_value);
                add_breakpoint(file, name, file->stabs[i].n_value, orig_info);
                ptrace(PTRACE_POKETEXT, pid, file->stabs[i].n_value, int3);
                //printf("orig_info = %x\n", orig_info);
                printf("(zdb)\tBreakpoint at addr %x set\n", file->stabs[i].n_value);
            }

        }
        i++;
    }
    return 0;
}

int return_from_bp(Elf32_file *file, pid_t pid)
{
    user_regs_struct regs;
    get_regs(pid, &regs);
    //printf("eip_prev = %x\n", regs.eip);
    regs.eip--;
    set_regs(pid, &regs);
    //printf("eip_cur = %x\n", get_eip(pid));

    put_orig_instr(file, pid);

    file->cur_bp->status = 0;
    //remove_breakpoint();
    return 0;
}

void put_orig_instr(Elf32_file *file, pid_t pid)
{
    Breakpoint *cur = file->breaks;
    while (cur)
    {
        if (cur->status)
            ptrace(PTRACE_POKETEXT, pid, cur->addr, cur->orig_info);
        cur = cur->next;
    }
}

void put_int3_instr(Elf32_file *file, pid_t pid)
{
    Breakpoint *cur = file->breaks;
    while (cur)
    {
        if (cur->status)
            ptrace(PTRACE_POKETEXT, pid, cur->addr, int3);
        cur = cur->next;
    }
}

Breakpoint *get_tail(Elf32_file *file)
{
    Breakpoint *cur = file->breaks;
    if (!cur)
        return NULL;
    while (cur->next)
    {
        cur = cur->next;
    }
    return cur;
}
