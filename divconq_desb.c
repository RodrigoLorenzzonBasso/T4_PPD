#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define DELTA_VALUE 100000
#define VETOR_SIZE 400000
//#define DEBUG 1

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

int *interleaving(int vetor[], int tam)
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

int main(int argc, char** argv)
{
    int my_rank;  // Identificador deste processo
    int proc_n;   // Numero de processos disparados pelo usuário na linha de comando (np)
    int delta = DELTA_VALUE;
    
    int * vetor = (int*)malloc(VETOR_SIZE * sizeof(int));
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
    }
    else
    {
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

    // dividir ou conquistar?

    if ( tam_vetor <= delta ) 
    {
        // conquistar
        bs(tam_vetor, vetor);
    }
    else
    {
        // dividir
        // quebrar em duas partes e mandar para os filhos

        MPI_Send (&vetor[0], tam_vetor/2, MPI_INT, my_rank*2+1, 0, MPI_COMM_WORLD);
        MPI_Send (&vetor[tam_vetor/2], tam_vetor/2, MPI_INT, my_rank*2+2, 0, MPI_COMM_WORLD);

        // receber dos filhos

        MPI_Recv (&vetor[0], tam_vetor/2, MPI_INT, my_rank*2+1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);            
        MPI_Recv (&vetor[tam_vetor/2], tam_vetor/2, MPI_INT, my_rank*2+2, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        // intercalo vetor inteiro

        vetor = interleaving(vetor, tam_vetor);
    }

    // mando para o pai

    if (my_rank !=0)
    {
        if(my_rank%2 == 0) // sou filho da direita
            MPI_Send(vetor, tam_vetor, MPI_INT, (my_rank-2)/2, 0, MPI_COMM_WORLD);
        else               // sou filho da esquerda
            MPI_Send(vetor, tam_vetor, MPI_INT, (my_rank-1)/2, 0, MPI_COMM_WORLD);
    }
    else
    {
        #ifdef DEBUG
        for(i = 0; i < tam_vetor; i++)
        {
            printf("%d ", vetor[i]);
        }
        printf("\n");
        #endif

        printf("time: %1.2f\n", MPI_Wtime() - time);

    }
        
    MPI_Finalize();
    return 0;
}