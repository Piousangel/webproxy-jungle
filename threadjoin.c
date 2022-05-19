#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

int result;

void *thread_main(void *arg)
{
    int n = *(int *)arg;
    int i;
    for(i = 0; i < n; i++){
        sleep(1);
        printf("Running...\n");
    }
    result = i;
    // return (void *)&result;
    pthread_exit((void *)&result);
}

int main(){

    pthread_t t_id;
    int arg = 10;
    int ret;
    int *a;

    ret = pthread_create(&t_id, NULL, thread_main, (void *) &arg);
    if(ret != 0){
        errno = ret;
        perror("thread_create");
        //errno
        return 0;
    }
    pthread_join(t_id, (void **)&a);
    printf("result : %d\n", *a);
   
    printf("End\n");
    return 0;
} 