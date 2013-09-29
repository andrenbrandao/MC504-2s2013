#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ANSI_COLOR_RED     "\033[22;31m"
#define ANSI_COLOR_GREEN   "\033[22;32m"
#define ANSI_COLOR_YELLOW  "\033[01;33m"
#define ANSI_COLOR_BLUE    "\033[22;34m"
#define ANSI_COLOR_MAGENTA "\033[22;35m"
#define ANSI_COLOR_CYAN    "\033[22;36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define NO_OF_CUSTOMERS 20
#define TEMPO 30000
#define TAM_MESA 45

pthread_t customers[NO_OF_CUSTOMERS];

int eating = 0, waiting = 0, sentando = 0;
int no_of_chairs, no_of_customers, n_espacos;

sem_t block;
pthread_mutex_t mutex;
int must_wait = 0;

/* exibição de estados dos clientes */
typedef enum {W, S, E, L} estado_t;
estado_t estado[NO_OF_CUSTOMERS];
sem_t sem_ref;

int posicao[NO_OF_CUSTOMERS];

void zera_posicoes() {
	int i;

	for(i=0; i<NO_OF_CUSTOMERS; i++) {
		posicao[i] = 0;
	}
}

void exibe_mesa(int client_id) {
	int i, j, encontrado = 0, sentando = 0;
	// system("clear");
	printf("\n\n");
	printf("                漢"ANSI_COLOR_RED"o"ANSI_COLOR_RESET"字         漢"ANSI_COLOR_RED"o"ANSI_COLOR_RESET"字          漢"ANSI_COLOR_RED"o"ANSI_COLOR_RESET"字         \n");                                            
	printf("         |‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾|\n");             
	printf("         |        "ANSI_COLOR_YELLOW"_______"ANSI_COLOR_RESET"            "ANSI_COLOR_YELLOW"_______"ANSI_COLOR_RESET"           |\n");   
	printf("         |     "ANSI_COLOR_YELLOW"-<|"ANSI_COLOR_GREEN"@@@@@@@"ANSI_COLOR_YELLOW"|"ANSI_COLOR_GREEN"@"ANSI_COLOR_YELLOW"|"ANSI_COLOR_RESET"      "ANSI_COLOR_YELLOW"-<|"ANSI_COLOR_GREEN"@@@@@@@"ANSI_COLOR_YELLOW"|"ANSI_COLOR_GREEN"@"ANSI_COLOR_YELLOW"|"ANSI_COLOR_RESET"        |\n");
	printf("         |     "ANSI_COLOR_YELLOW"-<|"ANSI_COLOR_GREEN"@@@@@@@"ANSI_COLOR_YELLOW"|"ANSI_COLOR_GREEN"@"ANSI_COLOR_YELLOW"|"ANSI_COLOR_RESET"      "ANSI_COLOR_YELLOW"-<|"ANSI_COLOR_GREEN"@@@@@@@"ANSI_COLOR_YELLOW"|"ANSI_COLOR_GREEN"@"ANSI_COLOR_YELLOW"|"ANSI_COLOR_RESET"        |\n");   
	printf("         |        "ANSI_COLOR_YELLOW"‾‾‾‾‾‾‾"ANSI_COLOR_RESET"            "ANSI_COLOR_YELLOW"‾‾‾‾‾‾‾"ANSI_COLOR_RESET"           |\n");   
	printf("         |");

	for(i=1; i<=45; i++){
		for(j=0; j<NO_OF_CUSTOMERS; j++) {
			if(estado[j]==E && (posicao[j]-10) == i) {
				printf(ANSI_COLOR_GREEN"@"ANSI_COLOR_RESET);
				encontrado = 1;
			}
		}
		if(!encontrado)
			printf("_");
		encontrado=0;
	}

	printf("|\n");

		/* checa se alguem esta sentando */
	for(j=0; j<NO_OF_CUSTOMERS; j++) {
		if(estado[j]==S)
			sentando = 1;
	}


	if(!sentando) {
		printf("          ");
		for(i=1; i<=45; i++){
			for(j=0; j<NO_OF_CUSTOMERS; j++) {
				if((posicao[j]-10) == i) {
					printf("C");
					encontrado = 1;
				}
			}
			if(!encontrado)
				printf(" ");
			encontrado=0;
		}
		printf("\n");
	}

}

void entra_sushibar(int client_id) {
	int i, j, k;      

	if(eating == 1) {
		/* posicao final do cliente (quanto maior, mais pra esquerda anda) */
		for(i=0; i<61-n_espacos; i++) {
			usleep(TEMPO);
			exibe_mesa(client_id); 

			/* posicao inicial de entrada (quanto maior, mais longe sai da fila) */
			for(j=i; j<70; j++) {
				printf(" ");
			}

			printf("C\n");
		}
		posicao[client_id] = j-i+1;
		// printf("Posicao: %d\n", posicao[client_id]);
	}
	else {
		/* posicao final do cliente (quanto maior, mais pra esquerda anda) */
		for(i=0; i<(71-n_espacos*eating-n_espacos); i++) {
			usleep(TEMPO);
			exibe_mesa(client_id); 

			/* posicao inicial do primeiro */
			for(j=(61-n_espacos); j<70; j++) {
				printf(" ");
			}
			printf("C");

			for(j=2; j<eating; j++) {
				for(k=0; k<n_espacos; k++) {
					printf(" ");
				}
				printf("C");
			}
			/* posicao inicial de entrada (quanto maior, mais longe sai da fila) */
			for(j=i; j<(70-eating*n_espacos); j++) {
				printf(" ");
			}

			printf("C\n");
		}
		posicao[client_id] = eating*(n_espacos+1) + 9;
		// printf("Posicao: %d\n", posicao[client_id]);
	}
}


void* sushi_bar(void* arg) { 
	int client_id = *(int *) arg;

	while(1){
		int i,n;

		pthread_mutex_lock(&mutex);

		if(must_wait) { 
			waiting+=1;

		/* CLIENTE ESPERANDO */
		// printf("waiting customer...%d\n", client_id);
			estado[client_id] = W;
			exibe_mesa(client_id);

		/* MUTEX LIBERADO */
			pthread_mutex_unlock(&mutex);

			sem_wait(&block);
			waiting -= 1;
		} 
		eating+=1;
		must_wait = (eating == no_of_chairs);

		estado[client_id] = S;
	// printf("-----Sitting customer...%d\n", client_id);
		entra_sushibar(client_id);

		if(eating == no_of_chairs) {
			printf("TABLE FULL WITH CUSTOMERS...\n");
		}

		estado[client_id] = E;
		exibe_mesa(client_id);


	/* MUTEX LIBERADO */
		if(waiting && !must_wait) 
			sem_post(&block);
		else
			pthread_mutex_unlock(&mutex);

	// printf("Eating customer...%d\n", client_id);
		sleep(3);

		pthread_mutex_lock(&mutex);
		eating-=1;
		estado[client_id] = L;
	/* CLIENTE SAINDO */
		zera_posicoes();
	// printf("Leaving customer...%d\n", client_id);
		exibe_mesa(client_id);

		if(eating == 0)
			must_wait = 0;

		if(waiting && !must_wait)
			sem_post(&block);
		else
			pthread_mutex_unlock(&mutex);

		sleep(3);
	} 
} 

void main() { 
	int i=0;
	char c;
	int customer_id[NO_OF_CUSTOMERS];

	printf("Insira o numero de cadeiras do Sushi Bar: ");
	scanf(" %d", &no_of_chairs);

	n_espacos = TAM_MESA/(no_of_chairs+1)-1; 

	for(i=0;i<NO_OF_CUSTOMERS;i++) {
		customer_id[i]=i;
		estado[i] = W;
	}

  //mutex=make_semaphore(1);
	pthread_mutex_init(&mutex,0);

  //block=make_semaphore(0);
	sem_init(&block,0,0);

	sem_init(&sem_ref, 0, 1);

	for(i=0;i<NO_OF_CUSTOMERS;i++) {
		pthread_create(&customers[i],0,sushi_bar,&customer_id[i]);
	}

  /* use system call to make terminal send all keystrokes directly to stdin */
//   system ("/bin/stty raw");
	while(c != 'q') {
		c = getchar();
	};
  /* use system call to set terminal behaviour to more normal behaviour */
//   system ("/bin/stty cooked");
} 