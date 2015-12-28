#include <stdio.h>
#include <sys/wait.h>

#include "elf.hpp"

#define MAXCMDLEN 10
#define EFLAGS_SIZE 32

#define zdb_print printf("(zdb)\t");printf



int main(int argc, char **argv)
{
	pid_t pid = 1;
	int status;

	if ((pid = fork()) == 0)
	{
		ptrace(PTRACE_TRACEME);
		execl(argv[1], argv[1], NULL);
	}
	else
	{
		Elf64_File elf(argc, argv, pid);
		wait(&status);
		elf.start();
		zdb_print("Debugging started\n");

		char *cmd = new char[MAXCMDLEN];
		char funcnm[100];

		while (!WIFEXITED(status))
		{
			elf.check_bp();

			zdb_print("ENTER A COMMAND:\n(zdb)\t");
			fgets(cmd, MAXCMDLEN, stdin);
			switch (cmd[0]) {
			case 's': {
				int count = 1;
				if (strlen(cmd) > 2)
					sscanf(cmd + 1, "%d", &count);
				for (int i = 0; i < count; i++) {
					ptrace(PTRACE_SINGLESTEP, pid, 0, 0);
					wait(&status);
					if (WIFEXITED(status))
						goto exit;
				}
			} break;
			case 'q':
				ptrace(PTRACE_KILL, pid);
				zdb_print("Exiting...\n");
				goto exit;
			case 'r':
//				print_regs(pid);
				break;
			case 'b':
				sscanf(cmd + 1, "%s", funcnm);
				zdb_print("Breakpoint at function %s\n", funcnm);
				elf.add_bp(funcnm, 0);
				break;
			case 'c':
				//				if (elf->state == STATE_BREAKP)
				//					put_int3_instr(elf, pid);
				ptrace(PTRACE_CONT, pid, 0, 0);
				wait(&status);
				break;
			case 'v':
				break;
			case 'e':
				elf.list_all_lines();
				break;
			case 'm':
				elf.get_current_instr();
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
