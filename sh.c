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
    char *command;
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

    return command;
}

// command line parser
// return argc and argv
struct CommandLineArg get_command_args(){

    char *command = get_command();

    // init argc and argv
    char **argv = NULL;
    int argc = 0;
    char *original_command = strdup(command);

    if(strcmp(command,"\n") != 0){

        // Remove potential end-of-line character(s)
        command[strcspn(command, "\n")] = 0;

        // command line parser
        char *token = strtok(command, " ");
        while(token != NULL){
            // use realloc since we dont know the argument size yet
            argv = realloc(argv, (argc+2) * sizeof(char*));

            int token_length = strlen(token);
            argv[argc] = (char *) malloc((token_length+1) * sizeof(char));
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
    cla.command = original_command;

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
    if(num_jobs != 0){
        fprintf(stderr, "Error: there are suspended jobs\n");
        return 1;
    }

    return 0;
}

int load_program(char *path, int argc, char **argv, char *full_command){

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

        // if there is no input file
        if(input_file == NULL){
            fprintf(stderr, "Error: invalid command\n");
            free(path);
            free_argv(argc,argv);
            exit(0);
        }
        int fd = open(input_file, O_RDONLY);
        if(fd < 0){ // If open fail, the file does not exist 
            fprintf(stderr, "Error: invalid file\n");
            free(path);
            free_argv(argc,argv);
            free(input_file);
            exit(0);
        }
        // STDIN = STDIN_FILENO = 0
        dup2(fd, STDIN_FILENO);
        close(fd);
        free(input_file);
    }

    // handle output redirection
    if(redirect_output){
        // if there is no output file
        if(output_file == NULL){
            fprintf(stderr, "Error: invalid command\n");
            free(path);
            free_argv(argc,argv);
            exit(0);
        }
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
    free_argv(argc,argv);
    exit(0);
    }
    else{
        // add WUNTRACED in order to support suspended jobs
        waitpid(pid, &status, WUNTRACED);

        // the returned status with WIFSTOPPED()
        if(WIFSTOPPED(status)){     //If true, add the job to the list of suspended jobs. 
            add_job(pid, full_command);
        }
    }

    return 0;
}

int check_pipe(int argc, char **argv, char *full_command){
    // check if pipe exist
    int num_commands = 0;
    char **commands = NULL; 
    char *pipeline = strdup(full_command);

    // split pipeline into separate commands
    char *token = strtok(pipeline, "|");
    while (token != NULL) {
        commands = realloc(commands, (num_commands+2) * sizeof(char*));
        int token_length = strlen(token);
        commands[num_commands] = (char *) malloc((token_length+1) * sizeof(char));
        strncpy(commands[num_commands], token, token_length);
        commands[num_commands][token_length] = '\0';
        num_commands++;
        commands[num_commands] = NULL;
        token = strtok(NULL, "|");
    }

    if(num_commands == 1){      // no pipe exist
        locate_program(argc,argv,full_command);
    }
    
    else{                      // pipe exist
        // create pipes
        int pipe_fds[num_commands-1][2];
        for(int i =0; i < num_commands-1; i++){
            if(pipe(pipe_fds[i]) == -1){
                perror("pipe");
                // free variable
                free(pipeline);
                free_argv(num_commands,commands);
                return -1;
            }
        }
        // load_program in each command
        for(int i =0; i<num_commands; i++){
            // Remove potential end-of-line character(s)
            commands[i][strcspn(commands[i], "\n")] = 0;

            //set up new_argc and new_argv
            char **new_argv = NULL;
            int new_argc = 0;
            char *each_full_command = strdup(commands[i]);

            // command line parser
            char *new_token = strtok(each_full_command, " ");
            while(new_token != NULL){
                // use realloc since we dont know the argument size yet
                new_argv = realloc(new_argv, (new_argc+2) * sizeof(char*));

                int new_token_length = strlen(new_token);
                new_argv[new_argc] = (char *) malloc((new_token_length+1) * sizeof(char));
                strncpy(new_argv[new_argc], new_token, new_token_length);

                new_argv[new_argc][new_token_length] = '\0';
                new_argc++;
                new_token = strtok(NULL, " ");
            }
            if(new_argc > 0){
                new_argv[new_argc] = NULL;
            }

            int pid_child = fork();
            if(pid_child == -1){
                perror("fork");
                free(each_full_command);
                free_argv(new_argc, new_argv);
                free(pipeline);
                free_argv(num_commands,commands);
                return -1;
            } else if(pid_child == 0){
                // set up stdin
                if(i > 0){
                    dup2(pipe_fds[i-1][0], STDIN_FILENO);
                }
                // set up stdin
                if(i < num_commands -1){
                    dup2(pipe_fds[i][1], STDOUT_FILENO);
                }
                for(int i = 0; i<num_commands-1;i++){
                    close(pipe_fds[i][0]);
                    close(pipe_fds[i][1]);
                }
                locate_program(new_argc,new_argv,commands[i]);
                free(each_full_command);
                free_argv(new_argc, new_argv);
                free(pipeline);
                free_argv(num_commands,commands);
                exit(0);
            }else{
                free(each_full_command);
                free_argv(new_argc, new_argv);
                //int status;
                //waitpid(pid_child, &status, 0);
            }
        }

        // close all pipe_fds
        for(int i = 0; i<num_commands-1;i++){
            close(pipe_fds[i][0]);
            close(pipe_fds[i][1]);
        }

        // wait for child processes to finish
        
        int status;
        for(int i = 0; i< num_commands;i++){
            wait(&status);
        } 

    }
    
    // free variable
    free(pipeline);
    free_argv(num_commands,commands);

    return 0;

}

int locate_program(int argc, char **argv, char *full_command){
    // calculate the number of slash
    int slash_count = 0;
    int str_len = strlen(argv[0]);
    while(str_len > 0){
        if(argv[0][str_len-1] == '/'){
            slash_count++;
        }
        str_len--;
    }

    // see if there is pipe passing by
    int pipe_count = 0;
    int pipe_len = strlen(full_command);
    while(pipe_len > 0){
        if(full_command[pipe_len-1] == '|'){
            fprintf(stderr, "Error: invalid command\n");
            return 1;
        }
        pipe_len--;
    }

    // absolute path start with /
    if(argv[0][0] == '/'){
        load_program(argv[0],argc,argv,full_command);
    }
    // relative path contains but not begin with /
    else if(slash_count > 0){
        load_program(argv[0],argc,argv,full_command);
    }
    // only the base name, run with /usr/bin
    else{
        char *newPath = malloc(strlen("/bin/") + strlen(argv[0]) + 1);
        strcpy(newPath,"/bin/");
        strcat(newPath, argv[0]);
        load_program(newPath,argc,argv,full_command);
        free(newPath);
    }

    return 0;

}


int my_jobs(int argc){
    // The jobs command takes no arguments.
    if(argc != 1){
        fprintf(stderr, "Error: invalid command\n");
    }
    print_jobs();

    return 0;

}

void add_job(pid_t pid, char* full_command){
    if(num_jobs < MAX_JOBS){
        jobs_list[num_jobs].pid = pid;
        jobs_list[num_jobs].job_command = strdup(full_command);
        num_jobs++;
    }
}

void remove_job(int index){
    if(index < 0 || index >= num_jobs ){
        return;
    }
    free(jobs_list[index].job_command);
    for(int i = index; i<num_jobs;i++){
        jobs_list[i] = jobs_list[i+1];
    }
    num_jobs--;
}

int my_fg(int argc, int index){
    // If fg is called with 0 or 2+ arguments
    if(argc != 2){
        fprintf(stderr, "Error: invalid command\n");
        return 1;
    }

    // If job index does not exist in the list of currently suspended jobs
    if(index < 0 || index >= num_jobs){
        fprintf(stderr, "Error: invalid job\n");
        return 1;
    }
    
    int status;
    char *job_command = strdup(jobs_list[index].job_command);
    int job_pid = jobs_list[index].pid;

    // Sending signal to continue the suspended job
    kill(job_pid, SIGCONT);

    // add WUNTRACED to trace if job finished or not
    waitpid(job_pid, &status,WUNTRACED);

    // remove the job from the job_list
    remove_job(index);

    // check the returned status with WIFSTOPPED()
    if(WIFSTOPPED(status)){     //If true, add the job back to the list of suspended jobs. 
        add_job(job_pid, job_command);
    }

    free(job_command);

    return 0;
}

void print_jobs(){
    for(int i = 0; i < num_jobs; i++){
        printf("[%d] %s", i+1, jobs_list[i].job_command);
    }
}

//implement signal handling
void signal_handler(){
    interrupt_flag = 1;
    return;
}

int shell(){

    // set it to unbuffer, cuz there is memcheck with printf
    setvbuf(stdout, NULL, _IONBF, 0);
    
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGTSTP, signal_handler);
    int go = 100;
    while(go > 1){
        // milestone 1 - print the prompt
        char *basename = print_basename();
        printf("[nyush %s]$ ", basename);
        fflush(stdout);
        

        // milestone 2 - get users input
        struct CommandLineArg commandArg = get_command_args();
        int argc = commandArg.argc;
        char **argv = commandArg.argv;
        char *full_command = commandArg.command;
 
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
            }else if(strcmp(argv[0],"jobs") == 0){
                my_jobs(argc);
            }else if(strcmp(argv[0],"fg") == 0){
                my_fg(argc, (int) atoi(argv[1])-1);   // -1 because we pass the index
            
            }else{
                //milestone 3 - ls
                //milestone 4
                check_pipe(argc,argv,full_command); // check_pipe then locate_program
            }

            // free variable
            free_argv(argc,argv);
        }

        free(full_command);
        go--;
    }
    return 0;
}
