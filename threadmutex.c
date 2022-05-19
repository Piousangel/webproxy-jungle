#include <stdio.h>
#include <pthread.h>
#include <errno.h> 
#include <unistd.h>

int count = 0;
pthread_mutex_t mutex;
// PTHREAD_COND_INITIALIZER init과정 대신 이걸 사용해도 됨
void *thread_main(void *arg)
{   
    int id = *(int *)arg;
    int i;
    
    for(i = 0; i < 300; i++){
        pthread_mutex_lock(&mutex);  //lock을 걸고 푸는 과정을 많이 해요
        count++;
        printf("id: %d, %d\n", id, count);
        pthread_mutex_unlock(&mutex);
    }
    
    return NULL;
}

int main(){

    pthread_t t_id1, t_id2, t_id3;
    int ret;
    int a = 1, b = 2, c = 3;

    pthread_mutex_init(&mutex, NULL);

    ret = pthread_create(&t_id1, NULL, thread_main, &a);
    ret = pthread_create(&t_id2, NULL, thread_main, &b);
    ret = pthread_create(&t_id3, NULL, thread_main, &c);
   
    pthread_join(t_id1, NULL);
    pthread_join(t_id2, NULL);
    pthread_join(t_id3, NULL);
    
    printf("count : %d\n", count);

    pthread_mutex_destroy(&mutex);
    return 0;
} 