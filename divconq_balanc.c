#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

#define DELTA_VALUE 100
#define VETOR_SIZE 400
#define DEBUG 1

void bs(int n, int * vetor);
void *interleaving(int vetor[], int tam);
void *mergesort(int vetor[], int tam_vetor, int level, int my_rank);
int get_level(int my_rank);

int main(int argc, char** argv)
{
    int my_rank;  // Identificador deste processo
    int proc_n;   // Numero de processos disparados pelo usuário na linha de comando (np)
    int level;
    int parent_rank;

    int * vetor = (int*)malloc(VETOR_SIZE * sizeof(int));
    int * vetor_aux;
    int tam_vetor;
    int i;
    double time;

    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n); 

    // recebo vetor

    if (my_rank != 0)
    {
        MPI_Recv(vetor, VETOR_SIZE, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_INT, &tam_vetor);  // descubro tamanho da mensagem recebida
        parent_rank = status.MPI_SOURCE; // descubro quem é meu pai
        level = get_level(my_rank); // descubro em que nível da arvore estou
    }
    else
    {
        level = 0;                  // raiz da arvore
        tam_vetor = VETOR_SIZE;      // defino tamanho inicial do vetor
        // sou a raiz e portanto gero o vetor
        for(i = 0; i < tam_vetor; i++)
        {
            vetor[i] = tam_vetor - i;

            #ifdef DEBUG
            printf("%d ", vetor[i]);
            #endif
        }
        #ifdef DEBUG
        printf("\n");
        #endif

        time = MPI_Wtime();
    }

    vetor_aux = mergesort(vetor, tam_vetor, level, my_rank);

    // manda para o pai
    if(my_rank != 0)
    {
        MPI_Send(&vetor_aux[0], tam_vetor, MPI_INT, parent_rank, 0, MPI_COMM_WORLD);
    }
    else // sou a raiz
    {
        #ifdef DEBUG
        for(i = 0; i < tam_vetor; i++)
        {
            printf("%d ", vetor_aux[i]);
        }
        printf("\n");
        #endif

        printf("time: %1.2f\n", MPI_Wtime() - time);
    }

    MPI_Finalize();
    return 0;
}

void bs(int n, int * vetor) {
  int c=0, d, troca, trocou =1;

  while (c < (n-1) & trocou ) {
    trocou = 0;
    for (d = 0 ; d < n - c - 1; d++)
      if (vetor[d] > vetor[d+1]) {
        troca = vetor[d];
        vetor[d] = vetor[d+1];
        vetor[d+1] = troca;
        trocou = 1;
      }
      c++;
  }
}

void *interleaving(int vetor[], int tam)
{
	int *vetor_auxiliar;
	int i1, i2, i_aux;

	vetor_auxiliar = (int *)malloc(sizeof(int) * tam);

	i1 = 0;
	i2 = tam / 2;

	for (i_aux = 0; i_aux < tam; i_aux++) {
		if (((vetor[i1] <= vetor[i2]) && (i1 < (tam / 2)))
		    || (i2 == tam))
			vetor_auxiliar[i_aux] = vetor[i1++];
		else
			vetor_auxiliar[i_aux] = vetor[i2++];
	}

	return vetor_auxiliar;
}

void *mergesort(int vetor[], int tam_vetor, int level, int my_rank)
{
    // dividir ou conquistar?
    if (tam_vetor <= DELTA_VALUE) 
    {
        // conquistar
        bs(tam_vetor, vetor);
        return vetor;
    }
    else
    {
        // dividir
        int rank_filho = my_rank + pow(2, level);

        // manda segunda metade
        MPI_Send(&vetor[tam_vetor/2], tam_vetor/2, MPI_INT, rank_filho, 0, MPI_COMM_WORLD);

        // chama recursivamente para primeira metade
        vetor = mergesort(&vetor[0], tam_vetor/2, level+1, my_rank);

        // recebe a segunda metade ordenada
        MPI_Status status;
        MPI_Recv(&vetor[tam_vetor/2], tam_vetor/2, MPI_INT, rank_filho, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        int * vetor_aux = interleaving(vetor, tam_vetor);
        free(vetor);
        return vetor_aux;
    }
}

int get_level(int my_rank)
{
    int level = 0;
    while (pow (2, level) <= my_rank)
        level++;
    return level;
}