#include "break.hpp"
#include "elf.hpp"

void Breakpoint::restore()
{
    ptrace(PTRACE_POKETEXT, parent->get_pid(), addr, orig_info);
}

void Breakpoint::unset()
{
	user_regs_struct tmp;
	memmove(&tmp, parent->get_regs(), sizeof(tmp));
	tmp.rip--;
	parent->set_regs(&tmp);
	restore();
	state = BP_DISABLED;
}
