#include <stdio.h>
#include <sys/wait.h>

#include "elf.h"

#define MAXCMDLEN 10
#define EFLAGS_SIZE 32

#define zdb_print printf("(zdb)\t");printf

void print_regs(pid_t pid)
{
    user_regs_struct regs;
    get_regs(pid, &regs);
    zdb_print("eax = %ld\nebx = %ld\necx = %ld\nedx = %ld\n",
            regs.orig_eax, regs.ebx, regs.ecx, regs.edx);
    zdb_print("EIP = %x\n", regs.eip);

    char eflags[EFLAGS_SIZE];
    long ef = regs.eflags;
    int i = EFLAGS_SIZE;
    while (--i >= 0)
    {
        eflags[i] = (ef & (1 << i)) >> i;
    }

    zdb_print("eflags = ");
    for (i = 0; i < EFLAGS_SIZE; i++)
    {
        printf("%d", eflags[i]);
    }
    printf("\n");
}

void get_cur_instr(pid_t pid)
{
    unsigned char m = ptrace(PTRACE_PEEKTEXT, pid, get_eip(pid));
    zdb_print("cur_cmd = %x\n", (unsigned) m);
}


int main(int argc, char **argv)
{
//    setbuf(stdout, NULL);
    pid_t pid = 1;
    int status;
    zdb_print("Enter filename: ");
    char fname[100];
    scanf("%s", fname);

    Elf32_file *myelf = open_elf(fname);

    if (!myelf)
    {
        zdb_print("File wasn't opened\n");
        return 1;
    }

    myelf->state = STATE_NOTSTARTED;
    get_sects(myelf);
    get_stab_sects(myelf);
    close(myelf->fd);

    if ((pid = fork()) == 0)
    {
        ptrace(PTRACE_TRACEME);
        execl(fname, fname, NULL);
    }
    else
    {
        wait(&status);
        myelf->state = STATE_STARTED;
        zdb_print("Debugging started\n");

        char *cmd = calloc(MAXCMDLEN, sizeof(*cmd));
        char funcnm[100];
        getchar();

        while (!WIFEXITED(status))
        {
            if (check_for_bp(myelf, pid))
            {
                myelf->state = STATE_BREAKP;
                return_from_bp(myelf, pid);
            }

            zdb_print("ENTER A COMMAND:\n(zdb)\t");
            fgets(cmd, MAXCMDLEN, stdin);
            switch (cmd[0])
            {
                case 'c':
                {
                    int count = 1;
                    if (strlen(cmd) > 2)
                        sscanf(cmd + 1, "%d", &count);
                    for (int i = 0; i < count; i++)
                    {
                        //zdb_print("%x\n", get_eip(pid));
                        ptrace(PTRACE_SINGLESTEP, pid, 0, 0);
                        wait(&status);
                        if (WIFEXITED(status))
                            goto exit;
                    }
                    break;
                }
                case 'q':
                    ptrace(PTRACE_KILL, pid);
                    zdb_print("Exiting...\n");
                    goto exit;
                    break;
                case 'r':
                    print_regs(pid);
                    break;
                case 'b':
                    sscanf(cmd + 1, "%s", funcnm);
                    zdb_print("Breakpoint at function %s\n", funcnm);
                    set_breakpoint(myelf, pid, funcnm, 0);
                    break;
                case 'g':
                    if (myelf->state == STATE_BREAKP)
                        put_int3_instr(myelf, pid);
                    ptrace(PTRACE_CONT, pid, 0, 0);
                    wait(&status);
                    break;
                case 'v':
                    break;
                case 'e':
                    list_all_lines(myelf);
                    break;
                case 'm':
                    get_cur_instr(pid);
                    break;
                default:
                    break;
            }
        }
    }
    exit:
//    fflush(stdout);
    wait(&status);
    return 0;
}
