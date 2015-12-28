#include "DebugInfo.hpp"

#include <iostream>
#include <algorithm>
#include <unistd.h>

static char *cut_func_name(char * const source)
{
    char *src = source;
    unsigned len = 0;
    while (*src != ':' && *src != 0)
    {
        len++, src++;
    }
    src = new char[len+1];
    memmove(src, source, len);
    return src;
}

int DebugInfo::fill_stabs(uint64_t num, long offset, long str_offset, int fd, int str_size)
{
	stabs_num = num;

	stabstr = new char[str_size];
	lseek(fd, str_offset, SEEK_SET);
	read(fd, stabstr, str_size);

	lseek(fd, offset, SEEK_SET);
	Stab tmp;
	for(int i = 0; i < stabs_num; i++) {
		tmp.read_stab(fd);
		char *nm = cut_func_name(stabstr + tmp.get_strx());
		Stab new_stab(tmp, nm);
		stabs.push_back(new_stab);
		delete[] nm;
	}

	return 0;
}

void DebugInfo::list_all_lines()
{
	for (auto stab: stabs) {
		if (stab.get_type() == N_FUN)
			printf("(zdb)\tFUNCTION: %s\n", stabstr + stab.get_strx());
		if (stab.get_type() == N_SLINE)
			printf("(zdb)\tLine num: %d\n", stab.get_desc());
	}
}

Stab *DebugInfo::find_stab(const char *name)
{
	auto it = std::find(stabs.begin(), stabs.end(), Stab(name));

	if(it == stabs.end()) {
		std::cerr << "Stab with name " << name << " not found\n";
		return nullptr;
	} else {
		return &(*it);
	}
}
