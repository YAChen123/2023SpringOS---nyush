#include <stdio.h>
#include <stdlib.h>  
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "sh.h"

#define MAX_JOBS 100

struct CommandLineArg{
    int argc;
    char **argv;
};

struct JobsObj{
    pid_t pid;
    char* job_command;
};

int interrupt_flag = 0;

struct JobsObj jobs_list[MAX_JOBS];
int num_jobs = 0;

// print the basename of the full current path
char *print_basename(){
    // get the full path
    char cwd[PATH_MAX+1];
    if(getcwd(cwd, sizeof(cwd)) == NULL){
        perror("getcwd() error");
    }
    // get the current directory
    char *last_slash = strrchr(cwd, '/');
    char *basename = NULL;

    // if in root directory, basename = '/'
    if(strcmp(cwd,last_slash) == 0){
        basename = malloc(2);
        strcpy(basename, "/");
    }else{
        basename =  malloc(strlen(last_slash ? last_slash + 1 : cwd)+1);
        strcpy(basename, last_slash ? last_slash + 1 : cwd);
    }

    return basename;
}

// get the user input using getline()
char *get_command(){
    char *command = NULL;
    size_t command_size = 0; 
    getline(&command, &command_size, stdin);

    if(strcmp(command,"\n") == 0){
        return command;
    }
     
    // Remove potential end-of-line character(s)
    command[strcspn(command, "\n")] = 0;

    return command;
}

// command line parser
// return argc and argv
struct CommandLineArg get_command_args(){

    char *command = get_command();
    // init argc and argv
    char **argv = NULL;
    int argc = 0;

    if(strcmp(command,"\n") != 0){

        // command line parser
        char *token = strtok(command, " ");
        while(token != NULL){
            // use realloc since we dont know the argument size yet
            argv = realloc(argv, (argc+2) * sizeof(char*));

            int token_length = strlen(token);
            argv[argc] = (char *) malloc(token_length+1 * sizeof(char));
            strncpy(argv[argc], token, token_length);

            argv[argc][token_length] = '\0';
            argc++;
            token = strtok(NULL, " ");
        }
        if(argc > 0){
            argv[argc] = NULL;
        }

    }
    // free variable
    free(command);

    // init struct CommandLineArg
    struct CommandLineArg cla;
    cla.argc = argc;
    cla.argv = argv;

    return cla;
}

int free_argv(int argc, char **argv){
    // free malloc
    for(int i = 0; i <= argc; i++){
        free(argv[i]);
    }
    free(argv);
    return 0;
}

int my_cd(int argc, char **argv){
    // 0 or 2+ arguments
    if(argc != 2){
        fprintf(stderr, "Error: invalid command\n");
        return -1;
    }

    // directory does not exist
    if(chdir(argv[1]) != 0){
        fprintf(stderr, "Error: invalid directory\n");
        return -1;
    }

    return 0;
}

int my_exit(int argc){
    // more than 0 arguments
    if(argc > 1){
        fprintf(stderr, "Error: invalid command\n");
        return 1;
    }

    return 0;
}

int load_program(char *path, int argc, char **argv){

    // create child process
    int pid = fork();
    int status;
    if(pid < 0){
        perror("fork() error");
    }else if(pid == 0){

    // handle redirection
    int redirect_input = 0;
    int redirect_output = 0;
    char *input_file = NULL;
    char *output_file = NULL;

    for(int i = 1; i < argc; i++){
        if(argv[i] == NULL){
            continue;
        }else if(strcmp(argv[i], "<") == 0){
            redirect_input = 1;
            input_file = argv[i+1];
            argv[i] = NULL;
            argv[i+1] = NULL;
        } else if(strcmp(argv[i], ">") == 0){
            redirect_output = 1;
            output_file = argv[i+1];
            argv[i] = NULL;
            argv[i+1] = NULL;
        } else if(strcmp(argv[i], ">>") == 0){
            redirect_output = 2;
            output_file = argv[i+1];
            argv[i] = NULL;
            argv[i+1] = NULL;
        }
    }
    // handle input redirection 
    if(redirect_input){
        int fd = open(input_file, O_RDONLY);
        if(fd < 0){ // If open fail, the file does not exist 
            fprintf(stderr, "Error: invalid file\n");
            free(path);
            free_argv(argc,argv);
            free(input_file);
            kill(getpid(), SIGKILL);
        }
        // STDIN = STDIN_FILENO = 0
        dup2(fd, STDIN_FILENO);
        close(fd);
        free(input_file);
    }

    // handle output redirection
    if(redirect_output){
        int fd;
        if(redirect_output == 1){
            // S_IRWXU = (S_IRUSR | S_IWUSR | S_IXUSR)
            fd = open(output_file, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU); 
        }else{
            fd = open(output_file, O_WRONLY|O_CREAT|O_APPEND, S_IRWXU);
        }

        // STDOUT = STDOUT_FILENO = 1
        dup2(fd, STDOUT_FILENO);
        close(fd);
        free(output_file);
    }

    execv(path,argv);
    fprintf(stderr, "Error: invalid program\n");
    free(path);
    free_argv(argc,argv);
    kill(getpid(), SIGKILL);
    }
    else{
        // add WUNTRACED in order to support suspended jobs
        waitpid(pid, &status, WUNTRACED);

        // the returned status with WIFSTOPPED()
        if(WIFSTOPPED(status)){     //If true, add the job to the list of suspended jobs. 
            // add_jobs here
        }
    }

    return 0;
}

int locate_program(int argc, char **argv){
    // calculate the number of slash
    int slash_count = 0;
    int str_len = strlen(argv[0]);
    while(str_len > 0){
        if(argv[0][str_len-1] == '/'){
            slash_count++;
        }
        str_len--;
    }

    // absolute path start with /
    if(argv[0][0] == '/'){
        load_program(argv[0],argc,argv);
    }
    // relative path contains but not begin with /
    else if(slash_count > 0){
        load_program(argv[0],argc,argv);
    }
    // only the base name, run with /usr/bin
    else{
        char *newPath = malloc(strlen("/bin/") + strlen(argv[0]) + 1);
        strcpy(newPath,"/bin/");
        strcat(newPath, argv[0]);
        load_program(newPath,argc,argv);
        free(newPath);
    }

    return 0;

}


int my_jobs(int argc, char **argv){
    // The jobs command takes no arguments.
    if(argc != 1){
        fprintf(stderr, "Error: invalid command\n");
    }

    return 0;

}

int add_job(pid_t pid, char* job_command){

}

//implement signal handling
void signal_handler(int signal){
    interrupt_flag = 1;
    printf("this is the signal numbber:%d\n", signal);
    return;
}

int shell(){

    // set it to unbuffer, cuz there is memcheck with printf
    setvbuf(stdout, NULL, _IONBF, 0);
    
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGTSTP, signal_handler);
    int go = 30;
    while(go > 1){
        // milestone 1 - print the prompt
        char *basename = print_basename();
        printf("[nyush %s]$ ", basename);
        fflush(stdout);
        

        // milestone 2 - get users input
        struct CommandLineArg commandArg = get_command_args();
        int argc = commandArg.argc;
        char **argv = commandArg.argv;
 
        // free variable
        free(basename);

        // The command exist
        if(argc > 0){

            //milestone 5 - cd <dir> and exit
            if(strcmp(argv[0],"cd") == 0){
                my_cd(argc,argv);
            }else if(strcmp(argv[0],"exit") == 0){
                if(my_exit(argc) == 0){
                    // this will break
                    go = -1;
                }
            }else{
                //milestone 3 - ls
                //milestone 4
                locate_program(argc,argv);
            }



            // free variable
            free_argv(argc,argv);
        }

        go--;


    }
    return 0;
}
