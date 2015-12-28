#include "stab.hpp"
#include <iostream>
#include <string.h>
#include <unistd.h>

int Stab::read_stab(int fd)
{
	read(fd, &n_strx, sizeof(n_strx));
	read(fd, &n_type, sizeof(n_type));
	read(fd, &n_other, sizeof(n_other));
	read(fd, &n_desc, sizeof(n_desc));
	read(fd, &n_value, sizeof(n_value));

	return 0;
}

bool operator==(const Stab &a, const Stab &b)
{
	return a.name == b.name;
}


