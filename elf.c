#include "elf.h"

Elf32_file *open_elf(char *name)
{
    int fd = open(name, O_RDONLY);
    if (fd < 0)
        return NULL;
    if (!is_elf(fd))
        return NULL;
    Elf32_file *myelf = calloc(1, sizeof(*myelf));
    myelf->fd = fd;
    myelf->name = name;
    myelf->breaks = NULL;
    return myelf;
}

void close_elf(Elf32_file *elf)
{
    close(elf->fd);
    free(elf);
}

int is_elf(const int fd)
{
    lseek(fd, 1, SEEK_SET);
    char check[3];
    read(fd, check, sizeof(check));
    lseek(fd, 0, SEEK_SET);
    if (check[0] != 'E' || check[1] != 'L' || check[2] != 'F')
        return 0;
    return 1;
}

void get_sects(Elf32_file *file)
{
    Elf32_Ehdr *hdr = calloc(1, sizeof(*hdr));
    lseek(file->fd, 0, SEEK_SET);
    read(file->fd, hdr, sizeof(*hdr));
    file->elfhdr = hdr;
    printf("(zdb)\tsetcs num = %d\n", file->elfhdr->e_shnum);
    lseek(file->fd, hdr->e_shoff, SEEK_SET);
    Elf32_Shdr *sects = calloc(hdr->e_shnum, sizeof(*sects));
    read(file->fd, sects, hdr->e_shnum * sizeof(*sects));
    file->sects = sects;

    Elf32_Shdr names_sect = file->sects[hdr->e_shstrndx];
    //lseek(file->fd, file->elfhdr->e_shstrndx, SEEK_SET);
    //read(file->fd, &names_sect, sizeof(names_sect));
    lseek(file->fd, names_sect.sh_offset, SEEK_SET);
    file->sect_names = calloc(names_sect.sh_size, sizeof(*(file->sect_names)));
    read(file->fd, file->sect_names, names_sect.sh_size);
    //printf("(zdb)\tsh_size = %d\n", names_sect.sh_size);
    //printf("%s\n", file->sect_names);
}

int get_stab_sects(Elf32_file *file)
{
    int stabs_ind = find_sect(file, ".stab");
    if (stabs_ind == -1)
    {
        //printf(".stabs section not found\n");
        return 2;
    }
    int stabstr_ind = find_sect(file, ".stabstr");
    if (stabstr_ind == -1)
    {
        //printf(".stabstr section not found\n");
        return 2;
    }
    int stabs_num = file->sects[stabs_ind].sh_size / sizeof(Stab);
    Stab *stabs = calloc(stabs_num, sizeof(*stabs));
    lseek(file->fd, file->sects[stabs_ind].sh_offset, SEEK_SET);
    read(file->fd, stabs, stabs_num * sizeof(*stabs));
    char *stabstr = calloc(file->sects[stabstr_ind].sh_size, sizeof(*stabstr));
    lseek(file->fd, file->sects[stabstr_ind].sh_offset, SEEK_SET);
    read(file->fd, stabstr, file->sects[stabstr_ind].sh_size);

    file->stabs = stabs;
    file->stabstr = stabstr;
    file->stabs_num = stabs_num;
    return 0;
}

int find_sect(const Elf32_file *file, const char *name)
{
    printf("(zdb)\tLooking for sect '%s'\n", name);
    fflush(stdout);
    for (int i = 0; i <= file->elfhdr->e_shnum; i++)
    {
        //printf("%s\n", file->sect_names + file->sects[i].sh_name);
        if (!strcmp(name, file->sect_names + file->sects[i].sh_name))
            return i;
    }
    return -1;
}

void list_all_lines(Elf32_file *file)
{
    for (int i = 1; i < file->stabs_num; i++)
    {
        if (file->stabs[i].n_type == N_FUN)
            printf("(zdb)\tFUNCTION: %s\n", file->stabstr + file->stabs[i].n_strx);
        if (file->stabs[i].n_type == N_SLINE)
            printf("(zdb)\tLine num: %d\n", file->stabs[i].n_desc);
    }
}

void get_regs(pid_t pid, user_regs_struct *regs)
{
    ptrace(PTRACE_GETREGS, pid, NULL, regs);
}

void set_regs(pid_t pid, user_regs_struct *regs)
{
    ptrace(PTRACE_SETREGS, pid, NULL, regs);
}

int get_eip(pid_t pid)
{
    user_regs_struct regs;
    ptrace(PTRACE_GETREGS, pid, NULL, &regs);
    return regs.eip;
}

int check_for_bp(Elf32_file *file, pid_t pid)
{
    printf("(zdb)\tChecking for bp...\n");
    int eip = get_eip(pid);
    Breakpoint *cur = file->breaks;
    while (cur)
    {
        printf("(zdb)\t0x%x --- 0x%x\n", eip, cur->addr + 1);
        if (eip == cur->addr + 1)
        {
            printf("(zdb)\tBreakpoint hit at addr 0x%x\n", cur->addr);
            file->cur_bp = cur;
            return 1;
        }
        cur = cur->next;
    }
    return 0;
}
