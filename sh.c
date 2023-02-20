#include <stdio.h>
#include <stdlib.h>  
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#include "sh.h"

int interrupt_flag = 0;

struct CommandLineArg{
    int argc;
    char **argv;
};

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
    int pid = fork();
    int status;
    if(pid < 0){
        perror("fork() error");
    }else if(pid == 0){
        execv(path,argv);
        fprintf(stderr, "Error: invalid program\n");
        free(path);
        free_argv(argc,argv);
        kill(getpid(), SIGKILL);
    }
    else{
        waitpid(pid, &status, 0);
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
        argv[0] = realloc(argv[0], strlen(newPath)+1);
        strcpy(argv[0],newPath);
        load_program(newPath,argc,argv);
        free(newPath);
    }

    return 0;

}

// implement signal handling
// void signal_handler(){
//     interrupt_flag = 1;
//     return;
// }

int shell(){
    
    //signal(SIGINT, signal_handler);
    int go = 10;
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
