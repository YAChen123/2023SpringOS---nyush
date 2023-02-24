#include <stdio.h>
#include <unistd.h>

int main(){
    int go = 1;
    while(go){
        printf("%d\n",go);
        sleep(1);
        go++;
    }
    return 0;
}

