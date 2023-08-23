#ifndef bst_h
#define bst_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define YES 1
#define NO 0

struct bst_node {
    int elem;
    struct bst_node *right;
    struct bst_node *left;
};

typedef struct bst_node bst_tree;

bst_tree * insert_bst(bst_tree *n, int e){
    if( n == NULL ){
        bst_tree *newTree = malloc(sizeof(bst_tree));
        if(newTree == NULL){
            printf("El elemento no se pudo insertar\n");
            return NULL;
        }
        newTree->elem = e;
        newTree->left = NULL;
        newTree->right = NULL;
        return newTree;
    }
    if( n->elem == e )
        return n;

    if( e > n->elem ){    // insert a la derecha
        n->right = insert_bst(n->right, e);
        return n;
    }
    // insert a la izquierda
    n->left = insert_bst(n->left, e);
    return n;    
}

int find_bst(bst_tree *n, int e){
    if(n == NULL)
        return NO;
    if( n->elem == e )
        return YES;
    if( e > n->elem )
        return find_bst(n->right, e);
    return find_bst(n->left, e);
}

void remove_bst(bst_tree **n, int e){
    if(*n == NULL)
        return;
    if( (*n)->elem == e ){
        if((*n)->right == NULL && (*n)->left == NULL){
            free(*n);
            *n = NULL;
            return;
        }
        if((*n)->right == NULL && (*n)->left != NULL){
            bst_tree *aux = *n;
            *n = (*n)->left;
            free(aux);
            return;
        }
        if((*n)->right != NULL && (*n)->left == NULL){
            bst_tree *aux = *n;
            *n = (*n)->right;
            free(aux);
            return;
        }
        if((*n)->right != NULL && (*n)->left != NULL){
            bst_tree *aux = (*n)->right;
            while(aux->left != NULL)
                aux = aux->left;
            (*n)->elem = aux->elem;
            remove_bst(&(*n)->right, aux->elem);
            return;
        }
    }
    if( e > (*n)->elem ){
        remove_bst(&(*n)->right, e);
        return;
    }
    remove_bst(&(*n)->left, e);
    return;
}

void free_bst(bst_tree *n){
    if(n == NULL)
        return;
    free_bst(n->left);
    free_bst(n->right);
    free(n);
}

void print_bst(bst_tree *n){
    if(n == NULL)
        return;
    print_bst(n->left);
    printf("%d ", n->elem);
    print_bst(n->right);
}

#endif /* bst_h */