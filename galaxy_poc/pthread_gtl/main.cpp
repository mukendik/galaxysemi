#include <QCoreApplication>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h> // needed for the time() function
#include <unistd.h> // not available on BCB5, but contains sleep()

/*
volatile int sv=10;
volatile int x,y,temp=10;
pthread_mutex_t mut;
pthread_cond_t con;

void *ChildThread1(void *arg)
{
    printf("ChildThread1\n");

    pthread_mutex_lock (&mut);
    //pthread_cond_wait(&con, &mut);
    x=sv;
    x++;
    sv=x;
    printf("\nThe child sv is %d\n",sv);
    pthread_mutex_unlock (&mut);
    //pthread_exit(NULL);
    return 0;
}

void *ChildThread2(void *arg)
{
    printf("\nChildThread2\n");

    pthread_mutex_lock (&mut);
    y=sv;
    y--;
    sv=y;
    printf("\nThe child2 sv is %d\n",sv);
    // pthread_cond_signal(&con);
    pthread_mutex_unlock (&mut);
    //pthread_exit(NULL);
    return 0;
}

int main(void)
{
    pthread_t child1,child2; //,child3;
    pthread_mutex_init(&mut, NULL);
    pthread_create(&child2,NULL,ChildThread2,NULL);
    pthread_create(&child1,NULL,ChildThread1,NULL);
    pthread_cond_destroy(&con);
    pthread_mutex_destroy(&mut);
    pthread_join(child1,NULL);
    pthread_join(child2,NULL);
    return 0;
}
*/








pthread_mutex_t count_mutex;
long long count;

long long get_count()
{
    long long c;
    pthread_mutex_lock(&count_mutex);
    sleep(1);
    c = count;
    pthread_mutex_unlock(&count_mutex);
    return (c);
}

void increment_count()
{
    pthread_mutex_lock(&count_mutex);
    sleep(1);
    //get_count(); // dead lock ?
    usleep(1000);
    count = count + 1;
    pthread_mutex_unlock(&count_mutex);
}


void* thread1(void* temp)
{
    while(true)
    {
        //usleep(1);
        increment_count();
        //printf("\nthread1: %lld\n", get_count());
    }
    return 0;
}

void* thread2(void* temp)
{
    while(true)
    {
        //usleep(1);
        //increment_count();
        long long c=get_count();
        printf("\nthread2: %lld\n", c);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    pthread_t thread_id1,thread_id2;

    int ret=pthread_create(&thread_id1,NULL,&thread1,NULL);
    printf("\npthread_create 1: %d\n", ret);
    ret=pthread_create(&thread_id2,NULL,&thread2,NULL);
    printf("\npthread_create 2: %d\n", ret);

    printf("\nJoining...\n");
    void *res=0;
    int r=pthread_join(thread_id1,&res);
    printf("First thread: %d\n", r);
    r=pthread_join(thread_id2,&res);
    printf("Second thread: %d\n", r);

    return a.exec();
}




/*
pthread_mutex_t read_mutex;
pthread_mutex_t write_mutex;

void* write(void *temp)
{
    printf("Writing...\n");
    char *ret=0;
    FILE *file1=0;
    char *str=0;
    pthread_mutex_lock(&write_mutex);
    sleep(5);
    pthread_mutex_lock(&read_mutex);
    printf("\nwrite: File locked, please enter the message:\n");
    str=(char *)malloc(10*sizeof(char));
    file1=fopen("temp.txt","w");
    if (!file1)
        return 0;
    scanf("%s",str);
    printf("Writing...\n");
    fprintf(file1,"%s",str);
    fclose(file1);
    pthread_mutex_unlock(&read_mutex);
    pthread_mutex_unlock(&write_mutex);
    printf("\nwrite: Unlocked the file. you can read it now.\n");
    return ret;
}


void *read(void *temp)
{
    printf("read: Reading thread...\n");
    char *ret=0;
    FILE *file1=0;
    char *str=0;
    pthread_mutex_lock(&read_mutex);
    printf("\nread: Sleeping...\n");
    sleep(5);
    pthread_mutex_lock(&write_mutex);
    printf("\nread: Opening file... \n");
    file1=fopen("temp.txt","r");
    if (!file1)
        return 0;
    str=(char*)malloc(10*sizeof(char));
    fscanf(file1,"%s",str);
    printf("\nread: Message from file is '%s' \n",str);

    fclose(file1);

    pthread_mutex_unlock(&write_mutex);
    pthread_mutex_unlock(&read_mutex);
    return ret;
}

int main()
{
    printf("main\n");
    pthread_t thread_id,thread_id1;
    pthread_attr_t attr;
    int ret=0;
    void *res=0;
    ret=pthread_create(&thread_id,NULL,&write,NULL);
    ret=pthread_create(&thread_id1,NULL,&read,NULL);
    printf("\n Created thread. Joining...\n");
    int r=pthread_join(thread_id,&res);
    printf("First thread: %d\n", r);
    r=pthread_join(thread_id1,&res);
    printf("Second thread: %d\n", r);
}
*/

