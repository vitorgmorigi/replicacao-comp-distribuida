#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <time.h>

typedef struct carro {
    int shifts;
    int topSpeed;
} carro_c;

int menuInicial() {
    printf("Insira a opcao desejada: \n");
    printf("1 - Cadastrar carro\n");
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

void escreveArquivo(FILE* arquivo, carro_c carro, char* filename, char* permissao){
   
    abreArquivoW(arquivo, filename, permissao);
    fwrite(&carro, sizeof(carro_c), 1, arquivo); 
    fclose(arquivo);
}

carro_c capturaDados(carro_c carro) {
   
    printf("Digite a qtd marchas: ");
    scanf("%d", &carro.shifts);
    printf("\n");

    printf("Digite a velocidade maxima: ");
    scanf("%d", &carro.topSpeed);
    printf("\n");

    return carro;
}

int main(int argc, char** argv){

    const int tag = 13;
    int opcaoEscolhida, rank, size;

    MPI_Init(&argc, &argv);
    MPI_Status status;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(size < 4){
	fprintf(stderr,"Requer pelo menos 4 processos livres. \n");
    }
    const int qtd = 2;
    int tamanhoBlocos[qtd];
    MPI_Datatype tipos[qtd];
    MPI_Datatype stat_type;
    MPI_Aint int_length, offsets[qtd];                 //Vetor que segmenta os offsets para cada variavel pertencente a struct *//

    tamanhoBlocos[0] = 1;
    tamanhoBlocos[1] = 1;

    tipos[0] = MPI_INT;
    tipos[1] = MPI_INT;

    offsets[0] = 0;
    offsets[1] = int_length;

    MPI_Type_create_struct(qtd, tamanhoBlocos, offsets, tipos, &stat_type);
    MPI_Type_commit(&stat_type);

    if(rank == 0){
	opcaoEscolhida = menuInicial();
    }

    MPI_Barrier(MPI_COMM_WORLD);
   
    if(rank == 0) {
   	 if(opcaoEscolhida == 1){
	
		carro_c carro;
		int shiftsx, topSpeeds;
		scanf("%d", &shiftsx);
		scanf("%d", &topSpeeds); 
	     
		carro.shifts = shiftsx;
		carro.topSpeed = topSpeeds;

		printf("tambem cheguei aqui %d hahahahah",rank);
		MPI_Send(&carro, sizeof(carro_c), stat_type, 1, tag, MPI_COMM_WORLD);
		printf("front-end enviando dados atraves de um buffer para o replica-manager-1\n");
	}
    } else {
	if (rank == 1) {
		carro_c carro;
		FILE* ptr;
		char* filename = ("replica-manager.txt");
		char* permissao = "a";
		const int origem = 1;
		
		MPI_Recv(&carro, 1, stat_type, origem, tag, MPI_COMM_WORLD, &status);
		escreveArquivo(ptr, carro, filename, permissao);
		printf("replica-manager-1 recebeu o buffer do front-end e adicionou o conteudo no banco-de-dados\n");

		int iterator;

		for(iterator = rank + 1; iterator <= 3; iterator++){
			MPI_Send(&carro, 1, stat_type, iterator, tag, MPI_COMM_WORLD);
			printf("replica-manager-%d replicou banco-de-dados para o replica-manager%d \n",rank,iterator); 	
		}
	} else if (rank > 1) {
		carro_c carro;
		FILE* ptr;
		char* filename;
		const int origem = 1;

		if(rank == 2){        	
			filename = ("replica-manager-2.txt");
		}else if(rank == 3){
			filename = ("replica-manager-3.txt");
		}
		char* permissao = "a";		
	
		MPI_Recv(&carro, 1, stat_type, origem, tag, MPI_COMM_WORLD, &status);
		escreveArquivo(ptr, carro, filename, permissao);
		printf("replica-manager-%d atualizado e consistente, backup realizado com sucesso\n",rank);
		}
	  }	

    MPI_Barrier(MPI_COMM_WORLD);   
    MPI_Type_free(&stat_type);
    MPI_Finalize();
    return 0;
}
