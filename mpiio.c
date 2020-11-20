#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <time.h>

typedef struct Pessoa {
    char nome[20];
    int idade;
    float altura;
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
    
    return arquivo;
}

void escreveArquivo(FILE* arquivo, pessoa_p pessoa, char* filename, char* permissao){
   
    abreArquivoW(arquivo, filename, permissao);
    fwrite(&pessoa, sizeof(pessoa_p), 1, arquivo); 
    fclose(arquivo);
}

void alocaMemoria(){

}

int* capturaDados() {
 
    pessoa_p pessoa;    

    int* buffer = (int*) malloc(sizeof(pessoa_p));    

    printf("Digite o nome: ");
    scanf("%s", pessoa.nome);
    printf("\n");

    printf("Digite a idade: ");
    scanf("%d", &pessoa.idade);
    printf("\n");

    printf("Digite a altura: ");
    scanf("%f", &pessoa.altura);
    printf("\n");

    memcpy(buffer, &pessoa, sizeof(pessoa_p));    

    return buffer;
}

int main(int argc, char** argv){

    const int tag = 13;
    int opcaoEscolhida, rank, size;

    MPI_Init(&argc, &argv);
    MPI_Status status;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_File file;
    char filename[128];

    const int qtd = 3;
    int tamanhoBlocos[qtd];
    MPI_Datatype tipos[qtd];
    MPI_Datatype stat_type;
    MPI_Aint char_length, int_length, offsets[qtd];                 //Vetor que segmenta os offsets para cada variavel pertencente a struct *//

    tamanhoBlocos[0] = 30; tamanhoBlocos[1] = 1; tamanhoBlocos[2] = 1;

    tipos[0] = MPI_CHAR; tipos[1] = MPI_INT; tipos[2] = MPI_DOUBLE;

    offsets[0] = 0; offsets[1] = char_length * 30; offsets[2] = char_length * 30 + int_length;

    MPI_Type_create_struct(qtd, tamanhoBlocos, offsets, tipos, &stat_type);
    MPI_Type_commit(&stat_type);

    if(rank == 0) {
	printf("processo %d entrou aqui\n",rank);
	opcaoEscolhida = menuInicial();
     	
   	 if(opcaoEscolhida == 1){
	
		printf("processo %d entrou aqui\n",rank);	
		int* buffer = capturaDados();
		int destino = rank + 1;
     
		printf("tambem cheguei aqui %d hahahahah",rank);
		MPI_Send(&buffer, 1, stat_type, destino, tag, MPI_COMM_WORLD);
		printf("front-end enviando dados atraves de um buffer para o replica-manager-1\n");
	 }
    } else {
	
	pessoa_p pessoa;
	int appBuffer[sizeof(pessoa_p)];
	
	if (rank == 1) {
		
		const int origem = 0;
		
		MPI_Recv(appBuffer, 1, stat_type, origem, tag, MPI_COMM_WORLD, &status);
		MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &file);
		MPI_File_write(file, appBuffer, sizeof(pessoa_p), stat_type, MPI_STATUS_IGNORE);
		MPI_File_close(&file);
		printf("replica-manager-1 recebeu o buffer do front-end e adicionou o conteudo no banco-de-dados\n");

		int iterator;

		for(iterator = rank + 1; iterator <= 3; iterator++){
			MPI_Send(&appBuffer, 1, stat_type, iterator, tag, MPI_COMM_WORLD);
			printf("replica-manager-%d replicou banco-de-dados para o replica-manager%d \n",rank,iterator); 	
		}
	} else if (rank > 1) {
		const origem = 1;
				
		MPI_Recv(appBuffer, 1, stat_type, origem, tag, MPI_COMM_WORLD, &status);
		MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &file);
		MPI_File_write(file, appBuffer, sizeof(pessoa_p), stat_type, MPI_STATUS_IGNORE);
		printf("replica-manager-%d atualizado e consistente, backup realizado com sucesso\n",rank);
		}
	  }	

    MPI_Barrier(MPI_COMM_WORLD);   
    MPI_Type_free(&stat_type);
    MPI_Finalize();
    return 0;
}
