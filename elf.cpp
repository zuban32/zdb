#include "elf.hpp"
#include <algorithm>

int is_elf(const int fd)
{
	lseek(fd, 1, SEEK_SET);
	char check[4];
	read(fd, check, 3);
	lseek(fd, 0, SEEK_SET);
	check[3] = 0;
	if (check[0] != 'E' || check[1] != 'L' || check[2] != 'F')
		return 0;
	return 1;
}

int Elf64_File::fill_sects()
{
	lseek(fd, 0, SEEK_SET);
	read(fd, &hdr, sizeof(hdr));
	printf("(zdb)\tsects num = %d\n", hdr.e_shnum);

	Elf64_Shdr tmp;

	lseek(fd, hdr.e_shoff, SEEK_SET);
	for(int i = 0; i < hdr.e_shnum; i++) {
		read(fd, &tmp, sizeof(tmp));
		sects.push_back(tmp);
	}

	Elf64_Shdr names_sect = sects[hdr.e_shstrndx];
	lseek(fd, names_sect.sh_offset, SEEK_SET);
	sect_names = new char[names_sect.sh_size];
	read(fd, sect_names, names_sect.sh_size);

	return 0;
}

int Elf64_File::fill_info()
{
	int stabs_ind = find_section(".stab");
	if (stabs_ind == -1)
	{
		fprintf(stderr, ".stabs section not found\n");
		return 2;
	}
	int stabstr_ind = find_section(".stabstr");
	if (stabstr_ind == -1)
	{
		fprintf(stderr, ".stabstr section not found\n");
		return 2;
	}
	uint64_t stabs_num = sects[stabs_ind].sh_size / STAB_SIZE;

	return info.fill_stabs(stabs_num, sects[stabs_ind].sh_offset, sects[stabstr_ind].sh_offset, fd, sects[stabstr_ind].sh_size);
}

int Elf64_File::find_section(const char *name)
{
	printf("(zdb)\tLooking for sect '%s'\n", name);
	fflush(stdout);
	for (int i = 0; i <= hdr.e_shnum; i++)
	{
		if (!strcmp(name, sect_names + sects[i].sh_name))
			return i;
	}
	return -1;
}

void Elf64_File::list_all_lines()
{
	info.list_all_lines();
}

user_regs_struct *Elf64_File::get_regs()
{
	ptrace(PTRACE_GETREGS, pid, NULL, &regs);
	return &regs;
}

void Elf64_File::set_regs(user_regs_struct *regs_)
{
	memmove(&regs, regs_, sizeof(*regs_));
	sync_regs();
}

void Elf64_File::sync_regs()
{
	ptrace(PTRACE_SETREGS, pid, NULL, &regs);
}

uint64_t Elf64_File::get_rip()
{
	return get_regs()->rip;
}

uint64_t Elf64_File::get_current_instr()
{
	uint64_t m = ptrace(PTRACE_PEEKTEXT, pid, get_rip());
//	zdb_print("cur_cmd = 0x%lx\n", m);
	return m;
}

void Elf64_File::check_bp()
{
	if(in_bp()) {
		state = STATE_BREAKP;
		cur_bp->unset();
	}
}

bool Elf64_File::in_bp()
{
	uint64_t rip = get_rip();
	uint32_t size = breaks.size();
	for(int i = 0; i < size; i++)
	{
		Breakpoint bp = breaks[i];
		uint64_t addr = bp.get_addr();
		if (rip == addr + 1 && bp.is_enabled())
		{
			printf("(zdb)\tBreakpoint hit at addr 0x%lx\n", bp.get_addr());
			cur_bp = &breaks[i];
			return true;
		}
	}
	return false;
}

int Elf64_File::add_bp(const char *name, uint32_t linenum)
{
	auto it = std::find(breaks.begin(), breaks.end(), Breakpoint(this, name, 0, 0, 0));
	if(it != breaks.end()) {
		std::cout << "(zdb) Breakpoint already exists\n";
		return 1;
	}

	Stab *stab = info.find_stab(name);

	if(stab != nullptr && stab->get_type() == N_FUN) {
		uint32_t addr = stab->get_value();
		uint64_t orig_info = ptrace(PTRACE_PEEKTEXT, pid, addr);
		ptrace(PTRACE_POKETEXT, pid, addr, (uint32_t)int3);
		printf("(zdb)\tBreakpoint at addr %x set\n", addr);
		Breakpoint bp(this, name, linenum, addr, orig_info);
		bp.set();
		breaks.push_back(bp);
		return 0;
	} else {
		std::cerr << "Function not found\n";
		return 2;
	}
}
