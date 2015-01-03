#ifndef BREAK_H_INCLUDED
#define BREAK_H_INCLUDED

#include <unistd.h>
#include <sys/ptrace.h>

#include "stab.h"
#include "common_defs.h"

#define int3 0xCC

int set_breakpoint(Elf32_file *file, pid_t pid, char *name, const unsigned numline);

int return_from_bp(Elf32_file *file, pid_t pid);

void put_orig_instr(Elf32_file *file, pid_t pid);

void put_int3_instr(Elf32_file *file, pid_t pid);

Breakpoint *get_tail(Elf32_file *file);

#endif // BREAK_H_INCLUDED
