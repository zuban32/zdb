#ifndef STAB_HPP_
#define STAB_HPP_

#define N_SO 0x64
#define N_SOL 0x84
#define N_FUN 0x24
#define N_SLINE 0x44

#define STAB_SIZE 12

#include <stdint.h>
#include <string>

class Stab
{
	uint32_t n_strx;
	uint8_t n_type;
	uint8_t n_other;
	uint16_t n_desc;
	uint32_t n_value;
	std::string name;

public:
	Stab() {}
	Stab(const char *nm): name(nm) {}
	Stab(Stab &s, const char *nm): n_strx(s.get_strx()),
									n_type(s.get_type()),
									n_other(s.get_other()),
									n_desc(s.get_desc()),
									n_value(s.get_value()),
									name(nm) {}
	~Stab() {}

	inline uint32_t get_strx() { return n_strx; }
	inline uint8_t get_type() { return n_type; }
	inline uint8_t get_other() { return n_other; }
	inline uint16_t get_desc() { return n_desc; }
	inline uint32_t get_value() { return n_value; }
	inline std::string get_name() { return name; }

	friend bool operator==(const Stab &a, const Stab &b);

	int read_stab(int fd);
};

#endif // STAB_HPP_
