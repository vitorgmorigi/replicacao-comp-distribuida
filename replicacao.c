#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <time.h>

typedef struct Pessoa {
    char nome[20];
    unsigned int idade;
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

void listaArquivo(){
}

void insere(FILE* ptr) {
    pessoa_p p;

    printf("Digite o nome: ");
    scanf("%19s", p.nome);
    printf("\n");

    printf("Digite a idade: ");
    scanf("%d", &p.idade);
    printf("\n");

    printf("Digite a altura: ");
    scanf("%f", &p.altura);
    printf("\n");

    fprintf(ptr, "[%s]\t[%d]\t[%.2f]\n", p.nome, p.idade, p.altura);
}

int main(int argc, char** argv){

    int opcaoEscolhida;

    opcaoEscolhida = menuInicial();

    if(opcaoEscolhida == 1){	
	
        MPI_Init(&argc, &argv);
    	MPI_Status status;

    	int rank, size;    

    	MPI_Comm_size(MPI_COMM_WORLD, &size);
    	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    	// Convertendo uma struct em C para o MPI, o MPI por padrao nao reconhece struct, ele nao eh nativo a linguagem C *//    

    	const int qtd = 3;
    	int tamanhoBlocos[qtd];
    	MPI_Datatype tipos[qtd];
    	MPI_Datatype stat_type;
    	MPI_Aint char_length, int_length, offsets[qtd];			//Vetor que segmenta os offsets para cada variavel pertencente a struct *//   
  
		
    	tamanhoBlocos[0] = 30;
	tamanhoBlocos[1] = 1;
	tamanhoBlocos[2] = 1;

    	tipos[0] = MPI_CHAR;
	tipos[1] = MPI_UNSIGNED;
	tipos[2] = MPI_DOUBLE;

	offsets[0] = 0; 
    	offsets[1] = char_length * 30;
	offsets[2] = char_length * 30 + int_length;
//	offsets[2] = (size_t)&(pessoa_p.altura) - (size_t)&pessoa_p;
    
    	MPI_Type_create_struct(qtd, tamanhoBlocos, offsets, tipos, &stat_type);
    	MPI_Type_commit(&stat_type);

    // Struct para MPI criada com sucesso //
	if(rank == 0) {
	
	pessoa_p pessoa;
         
	scanf("%s", pessoa.nome);
        scanf("%d", &pessoa.idade);
        scanf("%f", &pessoa.altura);
        
	MPI_Send(&pessoa, 1, stat_type, 1, MPI_ANY_TAG, MPI_COMM_WORLD);
	printf("front-end enviando dados atraves de um buffer para o replica-manager-1\n");
    } else {
	if (rank == 1) {
		pessoa_p pessoa;
		FILE* ptr;
		char* filename = ("replica-manager.txt");
		char* permissao = "a";
		
		MPI_Recv(&pessoa, 1, stat_type, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		escreveArquivo(ptr, pessoa, filename, permissao);
		printf("replica-manager-1 recebeu o buffer do front-end e adicionou o conteudo no banco-de-dados\n");

		int iterator;

		for(iterator = rank + 1; iterator < 3; iterator++){
			MPI_Send(&pessoa, 1, stat_type, iterator, MPI_ANY_TAG, MPI_COMM_WORLD);
			printf("replica-manager-%d replicou banco-de-dados para o replica-manager%d \n",rank,iterator); 	
		}
	} else if (rank > 1) {
		pessoa_p pessoa;
		FILE* ptr;
		char* filename;		

		if(rank == 2){        	
			filename = ("replica-manager-2.txt");
		}else if(rank == 3){
			filename = ("replica-manager-3.txt");
		}
		char* permissao = "a";		
	
		MPI_Recv(&pessoa, 1, stat_type, 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		escreveArquivo(ptr, pessoa, filename, permissao);
		printf("replica-manager-%d atualizado e consistente, backup realizado com sucesso\n",rank);
		}
  	}

	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Type_free(&stat_type);
	MPI_Finalize();
    }    

//    if ((ptr = fopen(filename, modo_gravacao)) == NULL) {
//        puts("Erro ao abrir o arquivo!");
//        return 1;
//    }

//    int opcaoEscolhida = menuInicial();

//    if(opcaoEscolhida == 1) {
//        insere(ptr);
//    } else if(opcaoEscolhida == 2) {
//        printf("Ainda nao implementado...");
//    } else {
//        printf("Ainda nao implementado...");
//    }


//    fclose(ptr);

//    puts("Arquivo gravado com sucesso!");

    return 0;
}
