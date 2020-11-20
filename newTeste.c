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

void Build_derived_type(pessoa_p* pessoa, MPI_Datatype* stat_type){

    const int qtd = 3;
    int tamanhoBlocos[qtd];
    MPI_Datatype tipos[qtd];
    MPI_Aint addresses[4];
    MPI_Aint offsets[qtd];                 //Vetor que segmenta os offsets para cada variavel pertencente a struct *//

    tipos[0] = MPI_CHAR; tipos[1] = MPI_INT; tipos[2] = MPI_DOUBLE;

    tamanhoBlocos[0] = 30; tamanhoBlocos[1] =  tamanhoBlocos[2] = 1;

    MPI_Address(&(pessoa->nome), &addresses[1]);
    MPI_Address(&(pessoa->idade), &addresses[2]);
    MPI_Address(&(pessoa->altura), &addresses[3]);

    offsets[0] = addresses[1] - addresses[0];
    offsets[1] = addresses[2] - addresses[0];
    offsets[2] = addresses[3] - addresses[0];

    MPI_Type_struct(qtd, tamanhoBlocos, offsets, tipos, &stat_type);
    MPI_Type_commit(&stat_type);
}

void get_data(pessoa_p* pessoa, int rank){
	MPI_Datatype message_type;
        printf("Insira nome, idade e altura \n");
        scanf("%s %d %f", &(pessoa->nome), &(pessoa->idade), &(pessoa->altura));
        Build_derived_type(pessoa, &message_type);

        printf("tambem cheguei aqui %d hahahahah",rank);
        MPI_Send(&pessoa, 1, stat_type, destino, tag, MPI_COMM_WORLD);
        printf("front-end enviando dados atraves de um buffer para o replica-manager-1\n");
}
int main(int argc, char** argv){

    const int tag = 13;
    int opcaoEscolhida;
    int rank;
    int size;

    MPI_Init(&argc, &argv);
    MPI_Status status;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == 0) {
	pessoa_p* pessoa;
	get_data(pessoa,rank);
    } else {

        pessoa_p pessoa;
        int appBuffer[sizeof(pessoa_p)];

        if (rank == 1) {

                FILE* ptr;
                char* filename = ("replica-manager.txt");
                char* permissao = "a";
                const int origem = 0;

                MPI_Recv(appBuffer, 1, stat_type, origem, tag, MPI_COMM_WORLD, &status);
                escreveArquivo(ptr, pessoa, filename, permissao);
                printf("replica-manager-1 recebeu o buffer do front-end e adicionou o conteudo no banco-de-dados\n");

                int iterator;

                for(iterator = rank + 1; iterator <= 3; iterator++){
                        MPI_Send(&appBuffer, 1, stat_type, iterator, tag, MPI_COMM_WORLD);
                        printf("replica-manager-%d replicou banco-de-dados para o replica-manager%d \n",rank,iterator);
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

                MPI_Recv(appBuffer, 1, stat_type, origem, tag, MPI_COMM_WORLD, &status);
                escreveArquivo(ptr, pessoa, filename, permissao);
                printf("replica-manager-%d atualizado e consistente, backup realizado com sucesso\n",rank);
                }
          }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Type_free(&stat_type);
    MPI_Finalize();
    return 0;
}

