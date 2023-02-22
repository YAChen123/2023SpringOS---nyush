#include <stdio.h>
#include <unistd.h>

int main(){
    int go = 0;
    while(go < 100){
        printf("%d\n",go);
        sleep(1);
        go++;
    }
    return 0;
}

