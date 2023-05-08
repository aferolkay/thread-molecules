#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

/*
	In this homework, I paid special attention to the
	naming of the variables. When reviewing my code please
	also consider the variable names as well as the comments
	to understand the code.

	Compile it with the following command:
	{
	"shell_cmd": "gcc -o PA1_part1 PA1_part1.c -pthread -lm -D_GNU_SOURCE"
	}

*/

#define OXYGEN 0
#define CARBON 1
#define NITROGEN 2
#define SULFUR 3
#define THORIAN 4

struct atom {
	int atomID;
	char atomTYPE; // C, N, S or Th
} atom_default = {-1,'X'};
typedef struct atom atom;

void * composer_CO2(void *arg);
void * composer_NO2(void *arg);
void * composer_SO2(void *arg);
void * composer_ThO2(void *arg);

bool productFlag=false; /* it is to let the main program know, a molecule generated	*/
int count = 0; /* how many atoms have been generated  */
int Mc=20,Mn=20,Ms=20,Mth=20,Mo=20,Grate=100,NUM_ATOMS_GENERATED=100;
char* informationVariable;
atom* atomArray; /* it holds the array where the generated atoms are placed*/

pthread_mutex_t atomMutex=PTHREAD_MUTEX_INITIALIZER; /* */
pthread_mutex_t syncCtoN,syncNtoC,syncCtoS, syncStoTh, syncThtoC;
pthread_mutex_t syncInfo , syncCompose;
pthread_t  CO2composer_thread , NO2composer_thread, SO2composer_thread, ThO2composer_thread;

time_t t;


/* Sole purpose of this function is to generate atoms randomly in accordance
	to the required number of generations for each atom type */
int randAtom(atom* atomArray){
	bool flag=0;
	int atomType;
	while(!flag){
		atomType = rand()%5;
		switch(atomType){
				case OXYGEN:
					if(Mo>0){
						flag=1;
						Mo--;
						atomArray[count].atomID = count;
						atomArray[count].atomTYPE = 'O';
					}
					break;
				case CARBON:
					if(Mc>0){
						flag=1;
						Mc--;
						atomArray[count].atomID = count;
						atomArray[count].atomTYPE = 'C';
					}
					break;
				case NITROGEN:
					if(Mn>0){
						flag=1;
						Mn--;
						atomArray[count].atomID = count;
						atomArray[count].atomTYPE = 'N';
					}
					break;
				case SULFUR:
					if(Ms>0){
						flag=1;
						Ms--;
						atomArray[count].atomID = count;
						atomArray[count].atomTYPE = 'S';
					}
					break;
				case THORIAN:
					if(Mth>0){
						flag=1;
						Mth--;
						atomArray[count].atomID = count;
						atomArray[count].atomTYPE = 'T';
					}
					break;
				default:
						printf("unknown element generated\n");
						exit(3);
					break;
			}
	}
	printf("%c with ID:%d is created.\n",atomArray[count].atomTYPE,atomArray[count].atomID );
	count++;
}


int main(int argc,char* argv[]){
	srand(10);
	//srand((unsigned) time(&t));
	int c;
	while((c = getopt(argc,argv,"c:n:s:t:o:g:"))!=-1){
		switch(c){
			case 'c':
				Mc=atoi(optarg);
				break;
			case 'n':
				Mn=atoi(optarg);
				break;
			case 's':
				Ms=atoi(optarg);
				break;
			case 't':
				Mth=atoi(optarg);
				break;
			case 'o':
				Mo=atoi(optarg);
				break;
			case 'g':
				Grate=atoi(optarg);
				break;
		}
	}
	/*necessary initializations*/
	NUM_ATOMS_GENERATED = Mc+Mn+Ms+Mth+Mo;
	atom atomArrayy[NUM_ATOMS_GENERATED];
	atomArray = atomArrayy;
	

	pthread_mutex_lock(&syncCtoN);
	pthread_mutex_lock(&syncNtoC);
	pthread_mutex_lock(&syncCtoS);
	pthread_mutex_lock(&syncStoTh);
	pthread_mutex_lock(&syncInfo);
	pthread_mutex_lock(&syncCompose);

	
    pthread_create(&CO2composer_thread, NULL, composer_CO2, NULL);
    pthread_create(&NO2composer_thread, NULL, composer_NO2, NULL);
    pthread_create(&SO2composer_thread, NULL, composer_SO2, NULL);
    pthread_create(&ThO2composer_thread, NULL, composer_ThO2, NULL);


    /* atomMutex protects the atomArray which stores all the atoms that have been generated or consumed, it
    needs to be protected because both consumers and producers can access and change to it.
    syncInfo and syncCompose makes sure that once a generation happens, the main thread will print it.
    */
	while ( ! (count >= NUM_ATOMS_GENERATED)){	
		pthread_mutex_lock(&atomMutex);
		if(count < NUM_ATOMS_GENERATED) randAtom(atomArray);
		pthread_mutex_unlock(&atomMutex);
		sleep( -1*log( 1-(double)rand()/RAND_MAX ) / Grate );
		if(productFlag){
			pthread_mutex_lock(&syncInfo);
			printf("%s",informationVariable);
			productFlag = false;
			pthread_mutex_unlock(&syncCompose);	
		}
	}
	
	pthread_join(CO2composer_thread, NULL);
    pthread_join(NO2composer_thread, NULL);
    pthread_join(SO2composer_thread, NULL);
    pthread_join(ThO2composer_thread, NULL);
	
	/*
	Once the generation of the molecules are over, leftover atoms are printed on the terminal.
	*/
	for (int i = 0; i < NUM_ATOMS_GENERATED ; i++){
		if(atomArray[i].atomTYPE != 'X')	{printf("%c with ID:%d is wasted.\n",atomArray[i].atomTYPE,atomArray[i].atomID );}
	}		
	return 0;
}


/* the comments I put on this composer applies to all composer threads since their structure is basically same*/
/* CO2 Composer */
void* composer_CO2(void *arg){
	/* these variables are necessary for which atoms have been generated or consumed */
	int C_counter=0;
	int Clocation=-1;
	int O_counter=0;
	int Olocation[2]={-1,-1};
	int state = 0; //describes wheter Th->C->N or N->C->S 
	bool innerFlag=false; /* describes whether a production occured or not but doesn need to be protected since only this thread uses it */

	while(1){
		//describes wheter Th->C->N or N->C->S 
		state ? pthread_mutex_lock(&syncNtoC) : pthread_mutex_lock(&syncThtoC);
		
		/* atomArray is bein read and written therefore it is protected with atomMutex 
		 	In this part, whether enough atoms exist to generate the molecule exist or not is being decided.*/
		pthread_mutex_lock(&atomMutex);
			for (int i = 0; i < NUM_ATOMS_GENERATED ; i++){
				if(atomArray[i].atomTYPE == 'C')	{C_counter=1; Clocation = i;}
				if(atomArray[i].atomTYPE == 'O')	{
					if(O_counter<2) Olocation[O_counter] = i;
					O_counter++;
				}
			}
			/*this block will execute if molecule generation is possible 
			consumed atoms will be deleted from the array
			necessary flags will be set
			*/
			if(C_counter>0 && O_counter>1){
				atomArray[Clocation] = atom_default;
				atomArray[Olocation[0]] = atom_default;
				atomArray[Olocation[1]] = atom_default;
				productFlag = true;
				innerFlag = true;
			}
		pthread_mutex_unlock(&atomMutex);
		/* This block is to set the information variable and make sure the main thread prints it if a 
			generation has occured. */
		if(innerFlag){
			innerFlag=false;
			informationVariable = "Composed molecule: CO2\n";
			pthread_mutex_unlock(&syncInfo);
			pthread_mutex_lock(&syncCompose);
			state ? pthread_mutex_unlock(&syncCtoS) : pthread_mutex_unlock(&syncCtoN);
			state ? (state = 0) : (state = 1);
		}
		else{ //describes wheter Th->C->N or N->C->S 
			state ? pthread_mutex_unlock(&syncNtoC) : pthread_mutex_unlock(&syncThtoC);
		}
		
		
	
		if(count >= NUM_ATOMS_GENERATED) { 
			
			pthread_mutex_unlock(&syncCtoN) ;
			pthread_exit(0);
			 }

		/* these variables are necessary for which atoms have been generated or consumed */	
		C_counter=0;
		Clocation=-1;
		O_counter=0;
		Olocation[0] = -1;
		Olocation[1] = -1;

	}	
}

/* NO2 Composer */	
void* composer_NO2(void *arg){
	int N_counter=0;
	int Nlocation=-1;
	int O_counter=0;
	int Olocation[2]={-1,-1};
	bool innerFlag=false;

	while(1){
		pthread_mutex_lock(&syncCtoN);
		
		pthread_mutex_lock(&atomMutex);
			for (int i = 0; i < NUM_ATOMS_GENERATED ; i++){
				if(atomArray[i].atomTYPE == 'N')	{N_counter=1; Nlocation = i;}
				if(atomArray[i].atomTYPE == 'O')	{
					if(O_counter<2) Olocation[O_counter] = i;
					O_counter++;
				}
			}
			if(N_counter>0 && O_counter>1){
				atomArray[Nlocation] = atom_default;
				atomArray[Olocation[0]] = atom_default;
				atomArray[Olocation[1]] = atom_default;
				productFlag = true;
				innerFlag=true;
			}
		pthread_mutex_unlock(&atomMutex);
		if(innerFlag)	{
			innerFlag=false;
			informationVariable = "Composed molecule: NO2\n";
			pthread_mutex_unlock(&syncInfo);
			pthread_mutex_lock(&syncCompose);
			pthread_mutex_unlock(&syncNtoC) ;
		}
		else
			pthread_mutex_unlock(&syncCtoN);
			
		
		if(count >= NUM_ATOMS_GENERATED) { 
			
			pthread_mutex_unlock(&syncCtoS) ;	
			pthread_exit(0);
			}  // all atoms are considered already
		N_counter=0;
		Nlocation=-1;
		O_counter=0;
		Olocation[0] = -1;
		Olocation[1] = -1;
		}
		
}

/* SO2 Composer */
void* composer_SO2(void *arg){
	int S_counter=0;
	int Slocation=-1;
	int O_counter=0;
	int Olocation[2]={-1,-1};
	bool innerFlag=false;

	while(1){
		pthread_mutex_lock(&syncCtoS);
		
		pthread_mutex_lock(&atomMutex);
			for (int i = 0; i < NUM_ATOMS_GENERATED ; i++){
				if(atomArray[i].atomTYPE == 'S')	{S_counter=1; Slocation = i;}
				if(atomArray[i].atomTYPE == 'O')	{
					if(O_counter<2) Olocation[O_counter] = i;
					O_counter++;
				}
			}
			if(S_counter>0 && O_counter>1){
				atomArray[Slocation] = atom_default;
				atomArray[Olocation[0]] = atom_default;
				atomArray[Olocation[1]] = atom_default;
				informationVariable = "Composed molecule: SO2\n";
				productFlag = true;
				innerFlag=true;
			}
		pthread_mutex_unlock(&atomMutex);
		if(innerFlag)	{
			innerFlag=false;
			informationVariable = "Composed molecule: SO2\n";
			pthread_mutex_unlock(&syncInfo);
			pthread_mutex_lock(&syncCompose);
			pthread_mutex_unlock(&syncStoTh) ;
		}
		else
			pthread_mutex_unlock(&syncCtoS) ;


		if(count >= NUM_ATOMS_GENERATED) { 
			pthread_mutex_unlock(&syncStoTh) ;
			printf("BURAYA BAKARLARRRRR SO\n");  
			pthread_exit(0);
			}  // all atoms are considered already
		S_counter=0;
		Slocation=-1;
		O_counter=0;
		Olocation[0] = -1;
		Olocation[1] = -1;
	}	
}

/* ThO2 Composer */
void* composer_ThO2(void *arg){
	int Th_counter=0;
	int Thlocation=-1;
	int O_counter=0;
	int Olocation[2]={-1,-1};
	bool innerFlag=false;

	while(1){
		pthread_mutex_lock(&syncStoTh);
		
		pthread_mutex_lock(&atomMutex);
			for (int i = 0; i < NUM_ATOMS_GENERATED ; i++){
				if(atomArray[i].atomTYPE == 'T')	{Th_counter=1; Thlocation = i;}
				if(atomArray[i].atomTYPE == 'O')	{
					if(O_counter<2) Olocation[O_counter] = i;
					O_counter++;
				}
			}
			if(Th_counter>0 && O_counter>1){
				atomArray[Thlocation] = atom_default;
				atomArray[Olocation[0]] = atom_default;
				atomArray[Olocation[1]] = atom_default;
				productFlag = true;
				innerFlag=true;
			}
			pthread_mutex_unlock(&atomMutex);
			if(innerFlag){
				innerFlag=false;
				informationVariable = "Composed molecule: ThO2\n";
				pthread_mutex_unlock(&syncInfo);
				pthread_mutex_lock(&syncCompose);
				pthread_mutex_unlock(&syncThtoC);
			}
			else{
				pthread_mutex_unlock(&syncStoTh); 
			}
			
		
		if(count >= NUM_ATOMS_GENERATED) { 

			pthread_mutex_unlock(&syncThtoC) ;
			pthread_mutex_unlock(&syncNtoC);
			printf("BURAYA BAKARLARRRRR Th\n"); 
			pthread_exit(0);
			}  // all atoms are considered already
		Th_counter=0;
		Thlocation=-1;
		O_counter=0;
		Olocation[0] = -1;
		Olocation[1] = -1;
	}	
}