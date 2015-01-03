#ifndef ELF_H_INCLUDED
#define ELF_H_INCLUDED

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>

#include "break.h"

Elf32_file *open_elf(char *name);

void close_elf(Elf32_file *elf);

int is_elf(const int fd);

void get_sects(Elf32_file *file);

int get_stab_sects(Elf32_file *file);

int find_sect(const Elf32_file *file, const char *name);

int check_for_bp(Elf32_file *file, pid_t pid);

void get_regs(pid_t pid, user_regs_struct *regs);

void set_regs(pid_t pid, user_regs_struct *regs);

int get_eip(pid_t pid);

void list_all_lines(Elf32_file *file);

#endif // ELF_H_INCLUDED
