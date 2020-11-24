Implementação utilizando à linguagem C do algoritmo de replicação passiva, combinado com a biblioteca para processamento paralelo MPI.

Comando para compilar o código:

```
gcc -o replicacao replicacao.c
```

Comando para rodar o código:

```
./replicacao
```

Comando para compilar com o MPI:

```
mpicc -o replicacao replicacao.c
```

Comando para rodar com o MPI:

```
mpirun -np 4 ./replicacao
```