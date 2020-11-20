#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <string.h>
#include <time.h>

typedef struct Pessoa {
    char nome[20];
    int idade;
} pessoa_p;

int menuInicial() {
    printf("Insira a opcao desejada: \n");
    printf("1 - Cadastrar usuario\n");
    printf("2 - Deletar usuario\n");
    printf("3 - Listar usuarios\n");
    
    int opcaoEscolhida;
    
    scanf("%d", &opcaoEscolhida);

    return opcaoEscolhida;
}

FILE* abreArquivoW(FILE* arquivo, char* filename, char* permissao){ 

    arquivo = fopen(filename,permissao); 
    printf("Arquivo Aberto com sucesso\n\n\n");
    return arquivo;
}

void escreveArquivo(FILE* arquivo, pessoa_p pessoa, char* filename, char* permissao){

   
    arquivo = abreArquivoW(arquivo, filename, permissao);
	fprintf(arquivo, "[%s]\t[%d]\n", pessoa.nome, pessoa.idade);
    fclose(arquivo);
    printf("Arquivo Escrito com Sucesso!!!!!\n\n\n");
}

pessoa_p capturaDados() {
 
    pessoa_p pessoa;    

    // int* buffer = (int*) malloc(sizeof(pessoa_p));    

    printf("Digite o nome: ");
    scanf("%s", pessoa.nome);
    printf("\n");

    printf("Digite a idade: ");
    scanf("%d", &pessoa.idade);
    printf("\n");

    // memcpy(buffer, &pessoa, sizeof(pessoa_p));    

    return pessoa;
}

int get_structMPI(){

	const int qtd = 2;
	
	int tamanhoBlocos[qtd]; tamanhoBlocos[0] = 30; tamanhoBlocos[1] = 1; // tamanhoBlocos[2] = 1;
	MPI_Datatype tipos[qtd]; tipos[0] = MPI_CHAR; tipos[1] = MPI_INT; //tipos[2] = MPI_FLOAT;
	MPI_Datatype stat_type;
	MPI_Aint offsets[qtd]; offsets[0] = 0; offsets[1] = sizeof(char) * 20; //offsets[2] = sizeof(char) * 30 + sizeof(int);
	MPI_Type_create_struct(qtd, tamanhoBlocos, offsets, tipos, &stat_type);
	MPI_Type_commit(&stat_type);

	return stat_type;
}

int main(int argc, char** argv){

    int rank,size,opcaoEscolhida;
    const int tag = 13;

    MPI_Init(&argc, &argv);
    MPI_Status status;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Datatype stat_type = get_structMPI();

    int appBuffer[sizeof(pessoa_p)];

    if(rank == 0) {

	int opcaoEscolhida;
    do {
        opcaoEscolhida = menuInicial();

        if (opcaoEscolhida == 1) {
			int destino = 1;
            pessoa_p pessoa = capturaDados();
            MPI_Send(&pessoa, 1, stat_type, destino, tag, MPI_COMM_WORLD);
			printf("front-end enviando dados atraves de um buffer para o replica-manager-1\n\n\n");
        }
        else if (opcaoEscolhida == 2) {
            printf("Ainda nao implementado...\n");
        }
        else if (opcaoEscolhida == 3) {
            printf("Ainda nao implementado...\n");
        }
        else {
            return 0;
        }
    } while (opcaoEscolhida >= 1 && opcaoEscolhida <= 3);


    } else {
	
	 pessoa_p pessoa;

	if (rank == 1) {
		FILE* ptr;
		char* filename = ("replica-manager.txt");
		char* permissao = "a";
		const int origem = 0;
	
		MPI_Recv(&pessoa, 1, stat_type, origem, tag, MPI_COMM_WORLD, &status);
		printf("Rank %d: Recebeu: nome = %s idade %d\n\n\n",rank, pessoa.nome, pessoa.idade);
		
		escreveArquivo(ptr, pessoa, filename, permissao);
		printf("replica-manager-1 recebeu o buffer do front-end e adicionou o conteudo no banco-de-dados\n\n\n");

		int iterator = 1;

		for(iterator = rank + iterator; iterator <= 3; iterator++){
			printf("Sou o processo %d e estou REPLICANDO para o processo %d \n\n\n",rank,iterator);
			MPI_Send(&pessoa, 1, stat_type, iterator, tag, MPI_COMM_WORLD);
			printf("replica-manager-%d replicou banco-de-dados para o replica-manager%d \n\n\n",rank,iterator); 	
		}
	} else if (rank > 1) {
		
		FILE* ptr;
		char* filename;
		const int origem = 1;

		if(rank == 2){        	
			filename = ("replica-manager-2.txt");
		}else if(rank == 3){
			filename = ("replica-manager-3.txt");
		}
		char* permissao = "a";
		
		MPI_Recv(&pessoa, 1, stat_type, origem, tag, MPI_COMM_WORLD, &status);
		escreveArquivo(ptr, pessoa, filename, permissao);
		printf("replica-manager-%d atualizado e consistente, backup realizado com sucesso\n\n\n",rank);
		}
	  }	

    MPI_Barrier(MPI_COMM_WORLD);   
    MPI_Type_free(&stat_type);
    MPI_Finalize();
    return 0;
}
