#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "bst.h"

int main(int argc, const char * argv[]){

    bst_tree *root = NULL;

    char line[100];
    char command[10];
    int number;

    while (fgets(line, sizeof(line), stdin) != NULL){
        sscanf(line, "%s %d", command, &number);
        if(strcmp(command, "insert") == 0) {
            root = insert_bst(root, number);
        }
        else if(strcmp(command, "lookup") == 0) {
            if (find_bst(root, number) == NO)
                printf("%d No Encontrado\n", number);
        }
        else if(strcmp(command, "delete") == 0) {
            remove_bst(&root, number);
        }
        else {
            printf("Comando no reconocido %s %d\n", command, number);
        }
    }

    //print_bst(root);

    free_bst(root);

    return 0;
}