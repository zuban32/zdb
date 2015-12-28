#ifndef BREAK_H
#define BREAK_H

#include <string>

#include <string.h>
#include <unistd.h>
#include <sys/ptrace.h>

#include "elf.hpp"
#include "stab.hpp"

#define BP_NEW			0x0
#define BP_ENABLED		0x1
#define BP_DISABLED		0x2

#define int3 0xCC

class Elf64_File;

class Breakpoint
{
	std::string func_name;
	uint32_t line_num;
	uint64_t addr;
	uint64_t orig_info;
	uint8_t state;
	Elf64_File *parent;

	void restore();

public:
	Breakpoint(Elf64_File *file, const char *name, uint32_t line, uint64_t addr, uint64_t orig_info): func_name(name)
	{
		parent = file;
		this->line_num = line;
		this->addr = addr;
		this->orig_info = orig_info;
		state = BP_NEW;
	}

	~Breakpoint() {}

	inline uint64_t get_addr() { return addr; }
	inline bool is_enabled() { return state == BP_ENABLED; }

	bool operator==(const Breakpoint &a) {
		return func_name == a.func_name;
	}

	inline void set() {
		state = BP_ENABLED;
	};
	void unset();
};

#endif // BREAK_H
