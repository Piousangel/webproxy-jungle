#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

void *thread_main(void *arg)
{
    int n = *(int *)arg;
    int i;
    for(i = 0; i < n; i++){
        sleep(1);
        printf("Running...\n");
    }
    return NULL;
}

int main(){

    pthread_t t_id;
    int arg = 3;
    int ret;

    ret = pthread_create(&t_id, NULL, thread_main, (void *) &arg);
    if(ret != 0){
        errno = ret;
        perror("thread_create");
        //errno
        return 0;
    }
 
    sleep(5);
    printf("End\n");
    return 0;
} 