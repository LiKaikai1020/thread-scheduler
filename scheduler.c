/**
 * 
 * File             : scheduler.c
 * Description      : This is a stub to implement all your scheduling schemes
 *
 * Author(s)        : @Kaikai Yang
 * Last Modified    : @date
*/

// Include Files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <time.h>

#include <math.h>
#include <pthread.h>


#define FCFS    0
#define SRTF    1
#define PBS     2
#define MLFQ    3
#define SEM_ID_MAX 50


typedef struct Thread{
	int tid;
	int timeQuanta;
	float currentTime;
	int remainingTime;
	int tprio;
}Th;

typedef struct readyQueue {
	Th th;
	pthread_cond_t cond;	
	struct readyQueue *next;
}readyQueue;

typedef struct semaphore{
	int value;
	struct readyQueue* L;
}sem;

/* forward declarations*/
void init_scheduler( int sched_type );
int schedule_me( float currentTime, int tid, int remainingTime, int tprio );
int P( float currentTime, int tid, int sem_id);
int V( float currentTime, int tid, int sem_id);

readyQueue* type_sched(float currentTime, int tid, int remainingTime, int tprio);
readyQueue* fcfs_sched(readyQueue* th);
readyQueue* pbs_sched(readyQueue* th);
readyQueue* srtf_sched(readyQueue* th);
readyQueue* mlfq_sched(readyQueue* th);
readyQueue* creat_thread_struct(float currentTime, int tid, int remainingTime, int tprio);
readyQueue* check_ready_queue(int tid);
readyQueue* check_block_queue(int sem_id);
void thread_sched(readyQueue* cur);

readyQueue* first = NULL;
pthread_mutex_t lock;
sem sem_arr[SEM_ID_MAX];
int schedType=-1;
int globalTime=-1; //the first thread will get CPU at globalTime=0
int i;





void init_scheduler( int sched_type ){
	schedType=sched_type;
	pthread_mutex_init(&lock, NULL);

	//initialize the semaphore value and Block queue
	for(i=0; i<SEM_ID_MAX; i++){
		sem_arr[i].value =0 ;
		sem_arr[i].L = NULL;
	}


}

int schedule_me( float currentTime, int tid, int remainingTime, int tprio ) {
	pthread_mutex_lock(&lock);
	readyQueue* cur=NULL;
	//ready queue	
	cur= type_sched(currentTime, tid, remainingTime, tprio);
	//main scheduler
	thread_sched(cur);
	
	globalTime ++;
	pthread_mutex_unlock(&lock);
	return globalTime;

}

int P( float currentTime, int tid, int sem_id) { // returns current global time
	
	pthread_mutex_lock(&lock);
	readyQueue* bq = NULL;
	readyQueue* cur= NULL;

	sem_arr[sem_id].value --;

	if(sem_arr[sem_id].value < 0){
		cur = first;
		first = first ->next;
		cur->next = NULL;
		
		if(sem_arr[sem_id].L == NULL){
			sem_arr[sem_id].L = cur;
		}
		else{	bq=sem_arr[sem_id].L;
			while(bq ->next != NULL)
				bq= bq->next;

			bq->next = cur;
		}
		//signal the next thread in ready queue
		pthread_cond_signal(&(first->cond));
	}

		
	pthread_mutex_unlock(&lock);
	return globalTime;
}

int V( float currentTime, int tid, int sem_id){ // returns current global time
	pthread_mutex_lock(&lock);
	readyQueue* cur = NULL;

	sem_arr[sem_id].value ++;
	if(sem_arr[sem_id].value <= 0){
		cur = sem_arr[sem_id].L;
		sem_arr[sem_id].L = sem_arr[sem_id].L ->next;
		cur -> next = NULL;

	//add to ready queue
	switch(schedType){
		case FCFS:
			cur=fcfs_sched(cur);
			break;
		case PBS:
			cur= pbs_sched(cur);
			break;
		case SRTF:	
			cur= srtf_sched(cur);
			break;
		case MLFQ:
			cur= mlfq_sched(cur);
			break;
	}
	}
	
	pthread_mutex_unlock(&lock);
	return globalTime;
}




/**
 * helper functions
*/

readyQueue* creat_thread_struct(float currentTime, int tid, int remainingTime, int tprio){
	readyQueue* new = malloc(sizeof(readyQueue));
	pthread_cond_init(&(new -> cond) , NULL);
	new -> th.currentTime = currentTime;
	new -> th.tid = tid;
	new -> th.remainingTime = remainingTime;
	new -> th.tprio = tprio;
	new -> th.timeQuanta = 0;
	new -> next = NULL;
	
	return new;

}

//A new thread coming in ready queue follows FCFS

readyQueue* type_sched(float currentTime, int tid, int remainingTime, int tprio){
	readyQueue* cur = NULL;
	
	cur= check_ready_queue(tid);
	// It is a new thread
	if(cur== NULL){
		cur= creat_thread_struct(currentTime, tid, remainingTime, tprio);
		switch(schedType){
			case FCFS:
			cur = fcfs_sched(cur);
			break;
			case MLFQ:
			cur = mlfq_sched(cur);
			break;
		}
	}
	else{
		cur -> th.currentTime = currentTime;
		cur -> th.remainingTime = remainingTime;
		//update time quanta for mlfq 
		cur->th. timeQuanta++; 

	}

	return cur;
}

readyQueue* fcfs_sched(readyQueue* th){
	readyQueue* temp = NULL;
		
	if(first == NULL){
		first = th; }	
	else{
		temp = first;
		while(temp -> next != NULL){		
			temp= temp->next;}
			
		temp ->next = th;
	}

	return th;
}
//A new thread coming in ready queue follows PBS
readyQueue* pbs_sched(readyQueue* th){

}

//A new thread coming in ready queue follows SRTF
readyQueue* srtf_sched(readyQueue* th){

}


readyQueue* first_1 =NULL;
//A new thread coming in ready queue follows MLFQ
readyQueue* mlfq_sched(readyQueue* th){
	readyQueue* temp= NULL;
	int time_qt = 5;
	// 1st level
	if(first == NULL)
		first =th;
	else{
		temp = first;
	       while(temp->next != NULL){
	       temp = temp ->next;}
       		temp->next = th;	       
	}	
	
	// 2nd level
	if(th->th.remainingTime > time_qt)
	{
		
		if(first_1 == NULL)
			first_1 =th;
		else{
		temp = first_1;
	       while(temp->next != NULL){
	       temp = temp ->next;}
       		temp->next = th;	       
		}	
	}	
	time_qt+=10;
	//3rd level
	if(th->th.remainingTime > time_qt)
	{
		
		temp = first_1;
	       while(temp->next != NULL){
	       temp = temp ->next;}
       		temp->next = th;	       
			
	}	
	time_qt+=15;
	//4th level
	if(th->th.remainingTime > time_qt){
		temp = first_1;
	       while(temp->next != NULL){
	       temp = temp ->next;}
       		temp->next = th;}
	time_qt+=20;
	//5th level
	if(th->th.remainingTime > time_qt){
		temp = first_1;
	       while(temp->next != NULL){
	       temp = temp ->next;}
       		temp->next = th;}
	time_qt+=25;
	//round robin	
	if(th->th.remainingTime > time_qt){
		temp = first_1;
	       while(temp->next != NULL){
	       temp = temp ->next;}
       		temp->next = th;}
	return th;
}


//Check if given tid is already in ready queue
readyQueue* check_ready_queue(int tid){
	readyQueue* cur = NULL;
	cur = first;
	

	while(cur != NULL){
		if(cur->th.tid == tid)
			return cur;
		else
			cur = cur -> next;

	}

	return cur;
}

//Thread schduler
void thread_sched(readyQueue* cur){
	// if the current thread is not the first thread in the ready queue
	while(cur->th.tid != first->th.tid){
		pthread_cond_wait(&(cur-> cond), &lock );
	}
	
	

	if(cur-> th.remainingTime == 0 ){
		//CPU brust for this thread has end,  delete this thread in ready queue
		first = first -> next;
		pthread_cond_destroy(&(cur->cond));
		free(cur);
		
		if(first == NULL)
			return ;
		else	
			pthread_cond_signal(&(first->cond));
		
		globalTime--;
		
	
	}

	//if current thread finish its timequanta at current level
	else if(((first->th.timeQuanta ==5)||(first->th.timeQuanta==15)||(first->th.timeQuanta==30)||(first->th.timeQuanta==50)||(first->th.timeQuanta==75))&&(schedType== 3)&&(first->th.remainingTime>0)){
		readyQueue *temp =NULL;
		temp=first;
			
		first = first ->next;
		pthread_cond_signal(&(first->cond));
		pthread_cond_wait(&(temp->cond),&lock);
		

	}
	//if the last thread at current level going to finish
	else if((first->next==NULL)&&(schedType==3)&&(first->th.remainingTime==1)){
		first->next=first_1;
	}


}
