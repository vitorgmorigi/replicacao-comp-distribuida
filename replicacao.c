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

void abreArquivo(){}

void fechaArquivo(){}

void listaArquivo(){}

void insere(FILE* ptr) {
    pessoa_p p;

    printf("Digite o nome: ");
    scanf("%s", &p.nome);
    printf("\n");

    printf("Digite a idade: ");
    scanf("%d", &p.idade);
    printf("\n");

    printf("Digite a altura: ");
    scanf("%f", &p.altura);
    printf("\n");

    fprintf(ptr, "[%s]\t[%d]\t[%.2f]\n", p.nome, p.idade, p.altura);
}

int main(int argc, char** argv)
{
    FILE* ptr;
    char* filename = "./arq_teste.txt";
    char* modo_gravacao = "a"; // modo de gravação "append" (adiciona no final)

    MPI_Init(&argc, &argv);
    MPI_Status status;

    int rank, size;

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Convertendo uma struct em C para o MPI, o MPI por padrao nao reconhece struct, ele nao eh nativo a linguagem C *//    

    const int qtd = 3;
    int qtdBlocos [3] = {1,1,1};
    MPI_Datatype tipos[3] = {MPI_CHAR,MPI_UNSIGNED,MPI_FLOAT};
    MPI_Datatype stat_type;
    MPI_Aint offsets[3];			//Vetor que segmenta os offsets para cada variavel pertencente a struct *//

    offsets[1] = offsetof(pessoa_p, nome);	//Shift bytes, passando a referencia da struct e o tamanho da variavel para cada bloco *//
    offsets[2] = offsetof(pessoa_p, idade);
    offsets[3] = offsetof(pessoa_p, altura);
    
    MPI_Type_create_struct(qtd, qtdBlocos, offsets, tipos, &stat_type);
    MPI_Type_commit(&stat_type);

    // Struct para MPI criada com sucesso //

    FILE* buffer = fopen(filename, modo_gravacao);

    if(rank == 0) {
	
	MPI_Send(&buffer, 1, stat_type, 1, MPI_ANY_TAG, MPI_COMM_WORLD);
	printf("front-end enviando dados atraves de um buffer para o replica-manager-1");
    } else {
	if (rank == 1) {
		MPI_Recv(&buffer, 1, stat_type, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		//funcao para adicionar o conteudo de buffer no arquivo de texto *//
		// abreArquivo();
		// escreveArquivo();
		printf("replica-manager-1 recebeu o buffer do front-end e adicionou o conteudo no banco-de-dados");

		int iterator;

		for(iterator = rank + 1; iterator < 3; iterator++){
			MPI_Send(&buffer, 1, stat_type, iterator, MPI_ANY_TAG, MPI_COMM_WORLD);
			printf("replica-manager-1 replicando o banco-de-dados para o replica-manager%d \n",iterator); 	
		}
	} else if(rank > 1) {

        int iterator;
		for(iterator = rank + 1; iterator < 3; iterator++) {
			MPI_Recv(&buffer, 1, stat_type, 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			//funcao para adicionar o conteudo de buffer no arquivo de texto-backup *//
			// abreArquivo();
			// escreveArquivo();
			printf("replica-manager atualizado e consistente, backup realizado com sucesso");
		}
	}
    }
    

    if ((ptr = fopen(filename, modo_gravacao)) == NULL) {
        puts("Erro ao abrir o arquivo!");
        return 1;
    }

    int opcaoEscolhida = menuInicial();

    if(opcaoEscolhida == 1) {
        insere(ptr);
    } else if(opcaoEscolhida == 2) {
        printf("Ainda nao implementado...");
    } else {
        printf("Ainda nao implementado...");
    }


    fclose(ptr);

    puts("Arquivo gravado com sucesso!");

    MPI_Finalize();
    return 0;
}
