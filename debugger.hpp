#ifndef DEBUGGER_HPP_
#define DEBUGGER_HPP_

#include <list>

class Debugger
{
	Elf64_File file;
	std::list<Breakpoint> breaks;

	Debugger() {}
	~Debugger() {
		delete breaks;
	}
};

#endif /* DEBUGGER_HPP_ */
