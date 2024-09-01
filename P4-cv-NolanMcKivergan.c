//Nolan McKivergan
//12285017
//CS 300-001

//This is part 2 of the office hours problem
//This solution uses mutexes and conditional variables to synchronize the threads

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
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

//Conditional Variables
pthread_cond_t student_cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t teacher_cv = PTHREAD_COND_INITIALIZER;

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
    for (int x = 0; x < student_num * 2; x++) {

        //Lock buffer initially
        printf("Teacher will call mutex_lock <seats_mutex>;\n");
        pthread_mutex_lock(&seats_mutex);

        //Wait until a student signals that they've entered the queue - will unlock buffer for waiting period
        while (count == 0) {
            printf("Teacher will call cond_wait <teacher_cv>;\n");
            pthread_cond_wait(&teacher_cv, &seats_mutex);
        }
        //At this point a student has entered the queue and the buffer becomes locked again

        //Get student from queue
        get();

        //Unlock buffer to allow students to continue using queue
        printf("Teacher will call mutex_unlock <seats_mutex>;\n");
        pthread_mutex_unlock(&seats_mutex);

        //Sleep to simulate helping student
        int sleep_time = mytime(left, right);
        printf("Teacher to sleep for %d sec;\n", sleep_time);
        sleep(sleep_time);
        printf("Teacher wake up;\n");

        //Signal student that help is complete
        printf("Teacher will call cond_signal <student_cv>;\n");
        pthread_cond_signal(&student_cv);
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

    //Repeat until the student has been helped twice
    while (help_count < 2) {

        //Wait until a seat becomes available
        while (count == student_num) {

            //Go study for a bit and then come back
            sleep_time = mytime(left, right);
            printf("Student <%lld> to sleep for %d sec;\n", thread_id, sleep_time);
            sleep(sleep_time);
            printf("Student <%lld> wake up;\n", thread_id);
        }

        //At this point there is space in the queue

        //Lock buffer
        printf("Student <%lld> will call mutex_lock <seats_mutex>;\n", thread_id);
        pthread_mutex_lock(&seats_mutex);

        put((int) thread_id);//Enter queue

        //Signal that a seat has been occupied
        printf("Student <%lld> will call cond_signal <teacher_cv>;\n", thread_id);
        pthread_cond_signal(&teacher_cv);

        //Wait for help to be complete before continuing - will unlock buffer for wait period
        printf("Teacher will call cond_wait <student_cv>;\n");
        pthread_cond_wait(&student_cv, &seats_mutex);

        //Unlock buffer
        printf("Student <%lld> will call mutex_unlock <seats_mutex>;\n", thread_id);
        pthread_mutex_unlock(&seats_mutex);        
        
        //Record help session
        help_count++;
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