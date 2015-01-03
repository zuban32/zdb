#ifndef COMMON_DEFS_H_INCLUDED
#define COMMON_DEFS_H_INCLUDED

#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <stdio.h>
#include <sys/user.h>
#include <sys/types.h>

#define STATE_NOTSTARTED 0x1230
#define STATE_STARTED 0x1231
#define STATE_BREAKP 0x1232
#define STATE_STOPPED 0x1233

typedef struct Breakpoint Breakpoint;
typedef struct user_regs_struct user_regs_struct;

struct Breakpoint
{
    char *func_name;
    unsigned line_num;
    unsigned addr;
    int orig_info;
    Breakpoint *next;
    char status;
};

typedef struct
{
    char *name;
    int fd;
    Elf32_Ehdr *elfhdr;
    Elf32_Shdr *sects;
    char *sect_names;
    Breakpoint *breaks;
    Breakpoint *cur_bp;
    Stab *stabs;
    char *stabstr;
    int stabs_num;
    int state;
} Elf32_file;

#endif // COMMON_DEFS_H_INCLUDED
