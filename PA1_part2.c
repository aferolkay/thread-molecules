#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include <semaphore.h>

/*
	In this homework, I paid special attention to the
	naming of the variables. When reviewing my code please
	also consider the variable names as well as the comments
	to understand the code.

	Compile it with the following command:
	{
	"shell_cmd": "gcc -o PA1_part2 PA1_part2.c -pthread -lm -D_GNU_SOURCE"
	}

*/


sem_t semaphoreC , semaphoreN , semaphoreS , semaphoreTh , semaphoreO ;
sem_t semaphoreCO2 , semaphoreNO2 , semaphoreSO2 , semaphoreThO2 ;
sem_t semAtom, semInfo, semCompose;
pthread_t CO2composer_thread , NO2composer_thread, SO2composer_thread, ThO2composer_thread,
			Cproducer_thread, Nproducer_thread, Sproducer_thread, Thproducer_thread , Oproducer_thread;


char* informationVariable; 
/* count is the current number of atoms that have been generated */
int count=0;
int Grate=100,M=10;


/* In all of the following produce threads, same comment for produce_C applies:
	semAtom is used for preservation of the count from the other generation threads.
	semaphoreC is used to signal the composers that a C atom is available.
	The thread will stop working once necessary amount of C is produced.
 */
void* produce_C(){
	int C=M;
	while(C!=0){
		sem_wait(&semAtom);
			count++;
			sem_post(&semaphoreC);
			printf("C with ID:%d is created.\n",count );
		sem_post(&semAtom);
		sleep( -1*log( 1-(double)rand()/RAND_MAX ) / Grate );
		C--;
	}
	pthread_exit(0);
}

void* produce_N(){
	int N=M;
	while(N!=0){
		sem_wait(&semAtom);
			count++;
			sem_post(&semaphoreN);
			printf("N with ID:%d is created.\n",count );
		sem_post(&semAtom);
		sleep( -1*log( 1-(double)rand()/RAND_MAX ) / Grate );
		N--;
	}
	pthread_exit(0);
}

void* produce_S(){
	int S=M;
	while(S!=0){
		sem_wait(&semAtom);
			count++;
			sem_post(&semaphoreS);
			printf("S with ID:%d is created.\n",count );
		sem_post(&semAtom);
		sleep( -1*log( 1-(double)rand()/RAND_MAX ) / Grate );
		S--;
	}
	pthread_exit(0);
}

void* produce_Th(){
	int Th=M;
	while(Th!=0){
		sem_wait(&semAtom);
			count++;
			sem_post(&semaphoreTh);
			printf("Th with ID:%d is created.\n",count );
		sem_post(&semAtom);
		sleep( -1*log( 1-(double)rand()/RAND_MAX ) / Grate );
		Th--;
	}
	pthread_exit(0);
}

void* produce_O(){
	int O=2*M;
	while(O!=0){
		sem_wait(&semAtom);
			count++;
			sem_post(&semaphoreO);
			printf("O with ID:%d is created.\n",count );
		sem_post(&semAtom);
		sleep( -1*log( 1-(double)rand()/RAND_MAX ) / Grate );
		O--;
	}
	pthread_exit(0);
}


/*  In all of the following composer threads, same comment for composer_CO2 applies:
	semaphoreCO2, semaphoreNO2, ... used for syncronization of the composers in the right order.
	state variable is introduced just for CO2 thread so that it can produce twice in single period.
	semaphoreC and semaphoreO is used to wait for necessary atoms to be available.
	The thread will stop working once necessary amount of C is produced.
	Once all the atoms are available and generation realize, it will signal the main after the
	information variable is updated.
 */
void* composer_CO2(){
	int state = 1;
	while(1){
		sem_wait(&semaphoreCO2);
		
			sem_wait(&semaphoreO);
			sem_wait(&semaphoreO);
			sem_wait(&semaphoreC);
			state = !state;
			informationVariable = "Composed molecule: CO2\n";
			sem_post(&semInfo);

		state ? sem_post(&semaphoreSO2):sem_post(&semaphoreNO2);
	}

}

void* composer_NO2(){

	while(1){
		sem_wait(&semaphoreNO2);
		
			sem_wait(&semaphoreO);
			sem_wait(&semaphoreO);
			sem_wait(&semaphoreC);
			informationVariable = "Composed molecule: NO2\n";
			sem_post(&semInfo);
			

		sem_post(&semaphoreCO2);
	}
}
void* composer_SO2(){
	while(1){

		sem_wait(&semaphoreSO2);
		
			sem_wait(&semaphoreO);
			sem_wait(&semaphoreO);
			sem_wait(&semaphoreC);
			informationVariable = "Composed molecule: SO2\n";
			sem_post(&semInfo);
			

		sem_post(&semaphoreThO2);
	}
}
void* composer_ThO2(){
	while(1){
		sem_wait(&semaphoreThO2);
		
			sem_wait(&semaphoreO);
			sem_wait(&semaphoreO);
			sem_wait(&semaphoreC);
			informationVariable = "Composed molecule: ThO2\n";
			sem_post(&semInfo);
			

		sem_post(&semaphoreCO2);
	}
}


int main(int argc,char* argv[]){
	srand(10);
	int c;
	while((c = getopt(argc,argv,"m:g:"))!=-1){
		switch(c){
			case 'm':
				M=atoi(optarg);
				M=M/6;
				break;
			case 'g':
				Grate=atoi(optarg);
				break;
		}
	}


	sem_init(&semaphoreC, 0, 0);
	sem_init(&semaphoreN, 0, 0);
	sem_init(&semaphoreS, 0, 0);
	sem_init(&semaphoreTh, 0, 0);
	sem_init(&semaphoreO, 0, 0);
	sem_init(&semaphoreCO2, 0, 1);
	sem_init(&semaphoreNO2, 0, 0);
	sem_init(&semaphoreSO2, 0, 0);
	sem_init(&semaphoreThO2, 0, 0);
	sem_init(&semAtom, 0, 1);
	sem_init(&semInfo, 0, 0);
	sem_init(&semCompose, 0, 0);

    pthread_create(&CO2composer_thread, NULL, composer_CO2, NULL);
    pthread_create(&NO2composer_thread, NULL, composer_NO2, NULL);
    pthread_create(&SO2composer_thread, NULL, composer_SO2, NULL);
    pthread_create(&ThO2composer_thread, NULL, composer_ThO2, NULL);

    pthread_create(&Cproducer_thread, NULL, produce_C, NULL);
    pthread_create(&Nproducer_thread, NULL, produce_N, NULL);
    pthread_create(&Sproducer_thread, NULL, produce_S, NULL);
    pthread_create(&Thproducer_thread, NULL, produce_Th, NULL);
    pthread_create(&Oproducer_thread, NULL, produce_O, NULL);

    /* until all the necessary atoms are produced, it will continue running
    	it will wait for a signal from the composers to print the information  */
    while(count < 6*M){
    	sem_wait(&semInfo);
    	printf("%s",informationVariable);
    }


    pthread_join(Oproducer_thread, NULL);
    pthread_join(Thproducer_thread, NULL);
    pthread_join(Sproducer_thread, NULL);
    pthread_join(Thproducer_thread, NULL);
    pthread_join(Oproducer_thread, NULL);

}