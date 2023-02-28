#include <stdio.h>

#include "sh.h"


int main(){
    return shell();
}



/*
---------------------------------------------Reference---------------------------------------------

Milestone 1:

    use of cwd():
    (https://stackoverflow.com/questions/298510/how-to-get-the-current-directory-in-a-c-program)

    Get the base name of my working directory:
    (https://en.cppreference.com/w/c/string/byte/strrchr)

Milestone 3:

    C comparing pointers (with chars)
    (https://stackoverflow.com/questions/7125697/c-comparing-pointers-with-chars)

    C - getline() and strcmp() issue
    (https://stackoverflow.com/questions/29397719/c-getline-and-strcmp-issue)

    structure
    (https://www.w3schools.com/c/c_structs.php)

    strtok() and strtok_r() functions in C with examples
    (https://www.geeksforgeeks.org/strtok-strtok_r-functions-c-examples/)

    realloca vs malloc
    (https://stackoverflow.com/questions/13350495/difference-between-malloc-and-realloc)

Milestone 5:

    chdir()
    (https://www.geeksforgeeks.org/chdir-in-c-language-with-examples/)

    check whether the file exist or not
    (https://codeforwin.org/c-programming/c-program-check-file-or-directory-exists-not)
    (https://www.geeksforgeeks.org/access-command-in-linux-with-examples/)

    execv()
    (https://support.sas.com/documentation/onlinedoc/ccompiler/doc/lr2/execv.htm)

    how to copy an char *str from an argv[0] without referencing each other
    (https://www.geeksforgeeks.org/strdup-strdndup-functions-c/)

    kill child process
    (https://stackoverflow.com/questions/7851121/can-i-kill-a-process-from-itself)

    printf memory leak (set to unbuffer)
    (https://stackoverflow.com/questions/55957454/heapusage-detects-memory-leak-possibly-caused-by-printf)

Signal Handling

    lecture slide page 91

Milestone 6: Input / output redirect

    how does input redirection work
    (https://askubuntu.com/questions/883786/how-does-input-redirection-work)

    dup2()
    (https://www.geeksforgeeks.org/dup-dup2-linux-system-call/)

    permission-denied-in-open-function-in-c
    (https://stackoverflow.com/questions/59602852/permission-denied-in-open-function-in-c)

    permission bits
    (https://www.gnu.org/software/libc/manual/html_node/Permission-Bits.html)

    Simulating the pipe "|" operator in C
    (https://www.youtube.com/watch?v=6xbLgZpOBi8&ab_channel=CodeVault)

Milestone 8

    how to split a string with a delimiter larger than one single char
    (https://stackoverflow.com/questions/2531605/how-to-split-a-string-with-a-delimiter-larger-than-one-single-char)

    Simulating the pipe "|" operator in C
    (https://www.youtube.com/watch?v=6xbLgZpOBi8&ab_channel=CodeVault)

Milestone 9

    Working with multiple pipes
    (https://www.youtube.com/watch?v=NkfIUo_Qq4c&ab_channel=CodeVault)

    How to make parent wait for all child processes to finish?
    (https://stackoverflow.com/questions/19461744/how-to-make-parent-wait-for-all-child-processes-to-finish)

    wc: standard input: Bad file descriptor on fork + pipe + execlp
    (https://stackoverflow.com/questions/37603297/wc-standard-input-bad-file-descriptor-on-fork-pipe-execlp)

Milestone 10:

    waitpid wuntraced
    (https://stackoverflow.com/questions/33508997/waitpid-wnohang-wuntraced-how-do-i-use-these)

    difference of sigstp and sigstop
    (https://stackoverflow.com/questions/11886812/what-is-the-difference-between-sigstop-and-sigtstp)

    char to int
    (https://www.geeksforgeeks.org/c-program-for-char-to-int-conversion/)

*/