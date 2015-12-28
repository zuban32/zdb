#ifndef DEBUGINFO_HPP_
#define DEBUGINFO_HPP_

#include <vector>
#include <string.h>

#include "stab.hpp"

class DebugInfo
{
    std::vector<Stab> stabs;
    char *stabstr;
    uint64_t stabs_num;

public:
    DebugInfo() {
    	memset(this, 0, sizeof(*this));
    }
    ~DebugInfo() {
    	delete []stabstr;
    }

    int fill_stabs(uint64_t num, long offset, long str_offset, int fd, int str_size);
    Stab *find_stab(const char *name);
    void list_all_lines();
};


#endif /* DEBUGINFO_HPP_ */
