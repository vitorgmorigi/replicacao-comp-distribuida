#include <stdio.h>
#include <stdlib.h>

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

int main(void)
{
    FILE* ptr;
    char* filename = "./arq_teste.txt";
    char* modo_gravacao = "a"; // modo de gravação "append" (adiciona no final)

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
    return 0;
}