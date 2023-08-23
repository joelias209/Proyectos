#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include "bst.h"

#define CRECIENTE 1
#define DECRECIENTE 2
#define ALEATORIO 3

void test_bst(int n, int caso){
    bst_tree *root = NULL;
    int *datos;

    if(caso == ALEATORIO){
        srand(time(NULL));
        datos = malloc(sizeof(int)*n); 
        for(int i = 0; i < n; i++){
            datos[i] = rand();
            datos[i] = datos[i]*RAND_MAX + rand(); 
        }
    }
    printf("%d\t", n);

    struct timeval begin, end;
    gettimeofday(&begin, NULL); 
    if(caso == ALEATORIO){
        for(int i=0; i<n; i++)
            root = insert_bst(root, datos[i]);
    }
    else{
        for(int i=0; i<n; i++)
            root = insert_bst(root, i);
    }
    gettimeofday(&end, NULL);

    long segundos = end.tv_sec - begin.tv_sec;
    long microsegundos = end.tv_usec - begin.tv_usec;
    double elapsed = (1e3*segundos + microsegundos*1e-3); // en ms    
    printf("%.3f\t", elapsed);

    gettimeofday(&begin, NULL); 
    if(caso == ALEATORIO){
        for(int i=0; i<n; i++){
            if(find_bst(root, datos[i]) == NO)
                printf("Elemento %d no encontrado!\n", datos[i]);
        }
    }
    else if(caso == CRECIENTE){
        for(int i=0; i<n; i++){
            if(find_bst(root, i) == NO)
                printf("Elemento %d no encontrado!\n", i);
        }
    }
    else{
        for(int i=0; i<n; i++){
            if(find_bst(root, n-i-1) == NO)
                printf("Elemento %d no encontrado!\n", n-i-1);
        }
    }
    gettimeofday(&end, NULL);

    segundos = end.tv_sec - begin.tv_sec;
    microsegundos = end.tv_usec - begin.tv_usec;
    elapsed = (1e3*segundos + microsegundos*1e-3); // en ms  
    printf("%.3f\t", elapsed);

    gettimeofday(&begin, NULL); 
    if(caso == ALEATORIO){
        for(int i=0; i<n; i++){
            remove_bst(&root, datos[i]);
        }
    }
    else if(caso == CRECIENTE){
        for(int i=0; i<n; i++){
            remove_bst(&root, i);
        }
    }
    else{
        for(int i=0; i<n; i++){
            remove_bst(&root, n-i-1);
        }
    }
    gettimeofday(&end, NULL);

    segundos = end.tv_sec - begin.tv_sec;
    microsegundos = end.tv_usec - begin.tv_usec;
    elapsed = (1e3*segundos + microsegundos*1e-3); // en ms    
    printf("%.3f\n", elapsed);

    free_bst(root);
}

int main(int argc, const char * argv[]){

    int n1 = 10000;
    int n2 = 100000;
    int n3 = 1000000;
    if(argc == 2){
        //sscanf(argv[1],"%d", &n);
    }

    printf("Numero de Datos\tInsercion\tBusqueda\tBorrado\n");
    for(int i=100; i<n1; i+=100)
        test_bst(i, ALEATORIO);

    for(int i=n1; i<n2; i+=1000)
        test_bst(i, ALEATORIO);

    for(int i=n2; i<=n3; i+=10000)
        test_bst(i, ALEATORIO);

    return 0;
}