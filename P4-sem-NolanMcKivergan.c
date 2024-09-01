//Nolan McKivergan
//12285017
//CS 300-001

//This is part 1 of the office hours problem
//This solution uses mutexes and semaphores to synchronize the threads

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include "mytime.h"

int seat_num;
int student_num;
int left, right;
int front = 0;
int rear = 0;
int queue_ptr = 0;

//Buffer (queue)
int *seats;
pthread_mutex_t seats_mutex = PTHREAD_MUTEX_INITIALIZER;
int count = 0;

//Buffer semaphores
sem_t empty_seats;
sem_t full_seats;
sem_t helping;

//Dequeue student thread from seats
int get() { 
    int temp = seats[front++];
    front = front % seat_num;
    count--;
    return temp;
} 

//Enqueue student thread into seats
void put (int c) { 	
    seats[rear++] = c;
    rear = rear % seat_num;
    count++;
}

void *teacher(void *arg) {
    //Repeat for 2*num of students
    for(int x = 0; x < student_num * 2; x++) {
        //Wait until someone enters the buffer
        printf("Teacher will call sem_wait <full_seats>;\n");
        sem_wait(&full_seats);

        //Lock buffer
        printf("Teacher will call mutex_lock <seats_mutex>;\n");
        pthread_mutex_lock(&seats_mutex);

        //Get student from queue
        get();

        //Unlock buffer
        printf("Teacher will call mutex_unlock <seats_mutex>;\n");
        pthread_mutex_unlock(&seats_mutex);

        //Post to empty seat semaphore to indicate seat has become available
        printf("Teacher will call sem_post <empty_seats>;\n");
        sem_post(&empty_seats);

        //Sleep to simulate help
        int sleep_time = mytime(left, right);
        printf("Teacher to sleep for %d sec;\n", sleep_time);
        sleep(sleep_time);

        //Post to semaphore to indicate student is done getting help
        printf("Teacher wake up;\n");
        sem_post(&helping);
    }
    pthread_exit(NULL);
}

void *student(void *arg) {
    long long int thread_id = (long long int) arg;
    int help_count = 0;//Number of times student has been helped

    //Initial sleep for random arrival time
    int sleep_time = mytime(left, right);
    printf("Student <%lld> to sleep for %d sec;\n", thread_id, sleep_time);
    sleep(sleep_time);

    //Repeat until student has been helped twice
    while (help_count < 2) {
        //Check if queue is full
        if (count == seat_num) {
            //Queue is full, try again later
            //Sleep
            sleep_time = mytime(left, right);
            printf("Student <%lld> to sleep for %d sec;\n", thread_id, sleep_time);
            sleep(sleep_time);
            printf("Student <%lld> wake up;\n", thread_id);
        }
        else {
            //Queue is not full
            //Wait until a seat becomes available
            printf("Student <%lld> will call sem_wait <empty_seats>;\n", thread_id);
            sem_wait(&empty_seats);

            //Unlock buffer
            printf("Student <%lld> will call mutex_lock <seats_mutex>;\n", thread_id);
            pthread_mutex_lock(&seats_mutex);

            //Enter queue
            put((int) thread_id);

            //Lock buffer
            printf("Student <%lld> will call mutex_unlock <seats_mutex>;\n", thread_id);
            pthread_mutex_unlock(&seats_mutex);

            //Post to semaphore indicating a seat has been filled
            printf("Student <%lld> will call sem_post <full_seats>;\n", thread_id);
            sem_post(&full_seats);

            //Wait until help is complete before continuing
            sem_wait(&helping);

            //Record help
            help_count++;
        }
    }
    pthread_exit(NULL);
}



int main(int argc, char *argv[]) {
    //Rand seed init
    srand(time(NULL));

    //Get user input
    if (argc != 5) {
		fprintf(stderr, "usage: %s <students> <chairs> <left> <right>\n", argv[0]);
		exit(1);
    }
    student_num = atoi(argv[1]);
    seat_num = atoi(argv[2]);
    left = atoi(argv[3]);
    right = atoi(argv[4]);

    //Set up seats buffer
    seats = (int *) malloc(seat_num * sizeof(int));
    for (int x = 0; x < seat_num; x++) {
		seats[x] = 0;
    }

    //Buffer semaphore init
    sem_init(&empty_seats, 0, seat_num);
    sem_init(&full_seats, 0, 0);
    sem_init(&helping, 0, 0);

    //Make thread for teacher
    pthread_t teacher_thread;
    pthread_create(&teacher_thread, NULL, teacher, NULL); 

    //Make threads for students
    pthread_t student_threads[student_num];
    for (int x = 0; x < student_num; x++) {
		pthread_create(&student_threads[x], NULL, student, (void *) (long long int) x); 
    }

    //Pause main execution until student threads are terminated
    for (int x = 0; x < student_num; x++) {
	    pthread_join(student_threads[x], NULL);
    }
    pthread_cancel(teacher_thread);
}