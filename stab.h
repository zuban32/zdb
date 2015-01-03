#ifndef STAB_H_INCLUDED
#define STAB_H_INCLUDED

#define N_SO 0x64
#define N_SOL 0x84
#define N_FUN 0x24
#define N_SLINE 0x44

typedef struct
{
    unsigned n_strx;
    unsigned char n_type;
    unsigned char n_other;
    unsigned short n_desc;
    unsigned n_value;
} Stab;

#endif // STAB_H_INCLUDED
