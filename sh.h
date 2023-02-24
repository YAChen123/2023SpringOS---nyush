#ifndef _SH_H_
#define _SH_H_

// some function
int shell();
char *print_basename();
char *get_command();
struct CommandLineArg get_command_args();
int load_program(char *path, int argc, char **argv, char *full_command);
int free_argv(int argc, char **argv);
int my_cd(int argc, char **argv);
int my_exit(int argc);
int locate_program(int argc, char **argv, char *full_command);
int my_jobs(int argc, char **argv);
void signal_handler(int signal);

#endif
