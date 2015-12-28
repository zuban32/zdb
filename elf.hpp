#ifndef ELF_H
#define ELF_H

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <elf.h>
#include <sys/user.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <list>
#include <vector>
#include <string>

#include "break.hpp"
#include "DebugInfo.hpp"

#define STATE_NOTSTARTED 	0x1230
#define STATE_STARTED 		0x1231
#define STATE_BREAKP 		0x1232
#define STATE_CONT			0x1233
#define STATE_STOPPED 		0x1234

class Breakpoint;

class Elf64_File
{
    std::string name;
    int fd;
    pid_t pid;
    Elf64_Ehdr hdr;
    std::vector<Elf64_Shdr> sects;
    char *sect_names;
    std::vector<Breakpoint> breaks;
    Breakpoint *cur_bp;

    int state;

    DebugInfo info;
    user_regs_struct regs;

public:
    Elf64_File(int argc, char **argv, pid_t pid): name(argv[1]) {
    	fd = open(name.c_str(), O_RDONLY);
    	state = STATE_NOTSTARTED;
    	this->pid = pid;
    	fill_sects();
    	fill_info();
    }

    ~Elf64_File() {
    	close(fd);
    	delete[] sect_names;
    }

    inline pid_t get_pid() { return pid; }

    inline void start() { state = STATE_STARTED; }

    int fill_sects();
    int fill_info();
    int find_section(const char *name);

    int add_bp(const char *name, const uint32_t linenum);
    void check_bp();
    bool in_bp();

    user_regs_struct *get_regs();
    void set_regs(user_regs_struct *regs);
    void sync_regs();
    uint64_t get_rip();

    uint64_t get_current_instr();

    void list_all_lines();
};

int is_elf(const int fd);

#endif // ELF_H
