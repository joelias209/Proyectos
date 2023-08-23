#ifndef splay_h
#define splay_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define YES 1
#define NO 0

struct splay_node {
    int elem;
    struct splay_node *right;
    struct splay_node *left;
    struct splay_node *parent;    
};

typedef struct splay_node splay_tree;

void splay_operation(splay_tree **n){
    if(*n == NULL)
        return;
    splay_tree *parent;
    splay_tree *grandparent;
    parent = (*n)->parent;
    if(parent == NULL)
        grandparent = NULL;
    else
        grandparent = (*n)->parent->parent;

    while(parent != NULL && grandparent != NULL){
        if((*n)->elem > (*n)->parent->elem){
            if((*n)->parent->elem > (*n)->parent->parent->elem){
                // Rotacion Right Zig–zig
                splay_tree *b = parent->left;
                splay_tree *c = (*n)->left;
                grandparent->right = parent->left;
                parent->right = (*n)->left;
                parent->parent = (*n);                        
                (*n)->parent = grandparent->parent;
                parent->left = grandparent;
                (*n)->left = parent;
                grandparent->parent = parent;
                if(b != NULL)
                    b->parent = grandparent;
                if(c != NULL)
                    c->parent = parent;
            }
            else{
                // rotacion Left Zig–zag
                splay_tree *b = (*n)->left;
                splay_tree *c = (*n)->right;
                grandparent->left = (*n)->right;
                parent->parent = (*n);                        
                (*n)->right = grandparent;
                parent->right = (*n)->left;
                (*n)->parent = grandparent->parent;
                grandparent->parent = (*n);
                (*n)->left = parent;    
                if(b != NULL)
                    b->parent = parent;   
                if(c != NULL)
                    c->parent = grandparent;              
            }
        }
        else{
            if((*n)->parent->elem > (*n)->parent->parent->elem){
                // Rotacion Right Zig–zag
                splay_tree *b = (*n)->left;
                splay_tree *c = (*n)->right;
                grandparent->right = (*n)->left;                        
                parent->left = (*n)->right;
                parent->parent = (*n);                        
                (*n)->left = grandparent;
                (*n)->parent = grandparent->parent;
                grandparent->parent = (*n);
                (*n)->right = parent;
                if(b != NULL)
                    b->parent = grandparent;
                if(c != NULL)
                    c->parent = parent;
            }
            else{
                // rotacion Left Zig–zig
                splay_tree *b = (*n)->right;
                splay_tree *c = parent->right;
                grandparent->left = parent->right;
                parent->left = (*n)->right;
                parent->parent = (*n);
                (*n)->right = parent;
                (*n)->parent = grandparent->parent;
                grandparent->parent = parent;
                parent->right = grandparent;
                if(b != NULL)
                    b->parent = parent;
                if(c != NULL)
                    c->parent = grandparent;
            }
        }
        parent = (*n)->parent;
        if(parent == NULL)
            break;
        grandparent = (*n)->parent->parent;
    }
    if(parent == NULL)
        return;
    if(grandparent == NULL){
        if((*n)->elem > parent->elem){
            //rotacion izquierda
            parent->parent = (*n);
            parent->right = (*n)->left; 
            if((*n)->left != NULL)
                (*n)->left->parent = parent;
            (*n)->parent = NULL;
            (*n)->left = parent;                                       
            return;
        }
        else{
            //rotacion derecha                    
            parent->parent = (*n);
            parent->left = (*n)->right;
            if((*n)->right != NULL)
                (*n)->right->parent = parent;
            (*n)->parent = NULL;
            (*n)->right = parent;
            return;
        }
    }
}

void insert_splay(splay_tree **n, int e){
    splay_tree *parent = NULL;
    while(*n != NULL){
        if((*n)->elem == e){
            splay_operation(n);
            return;
        }
        if(e > (*n)->elem){
            parent = *n;
            *n = (*n)->right;
        }
        else{
            parent = *n;
            *n = (*n)->left;
        }
    }
    *n = malloc(sizeof(splay_tree));
    if(*n == NULL){
        printf("El elemento no se pudo insertar\n");
        return;
    }
    (*n)->elem = e;
    (*n)->left = NULL;
    (*n)->right = NULL;
    (*n)->parent = parent;
    splay_operation(n);
    return;
}

int find_splay(splay_tree **n, int e){
    if(*n == NULL)
        return NO;
    splay_tree *parent;
    while(*n != NULL){
        if( (*n)->elem == e ){
            splay_operation(n);
            return YES;
        }
        if( e > (*n)->elem ){
            parent = (*n);
            *n = (*n)->right;
        }
        else{
            parent = (*n);
            *n = (*n)->left;
        }
    }
    *n = parent;
    splay_operation(n);
    return NO;
}
/*
void remove(splay_tree **n, int e){
    splay_tree *aux = (*n)->left;
    while(aux->right != NULL)
        aux = aux->right;
    (*n)->elem = aux->elem;
    aux->elem = aux->left->elem;
    aux->right = aux->left->right;
    aux->left = aux->left->left;
    free(aux->left);
}*/

void remove_splay(splay_tree **n, int e){
    if(find_splay(n, e) == NO)
        return;
    if((*n)->right == NULL && (*n)->left == NULL){
        free(*n);
        *n = NULL;
        return;
    }
    if((*n)->right == NULL && (*n)->left != NULL){
        splay_tree *aux = *n;
        *n = (*n)->left;
        (*n)->parent = NULL;
        free(aux);
        return;
    }
    if((*n)->right != NULL && (*n)->left == NULL){
        splay_tree *aux = *n;
        *n = (*n)->right;
        (*n)->parent = NULL;
        free(aux);
        return;
    }
    if((*n)->right != NULL && (*n)->left != NULL){
        splay_tree *aux = (*n)->left;
        while(aux->right != NULL)
            aux = aux->right;
        (*n)->elem = aux->elem;
        if(aux == (*n)->left){
            (*n)->left = aux->left;
            if(aux->left != NULL)
                aux->left->parent = (*n);
            free(aux);
            return;
        }
        splay_tree *parent = aux->parent;
        parent->right = aux->left;
        if(aux->left != NULL)
            aux->left->parent = parent;
        free(aux);
    }    
}

void free_splay(splay_tree *n){
    if(n == NULL)
        return;
    free_splay(n->left);
    free_splay(n->right);
    free(n);
}

void print_splay(splay_tree *n){
    if(n == NULL)
        return;
    print_splay(n->left);
    printf("%d ", n->elem);
    print_splay(n->right);
    if(n->parent == NULL)
        printf("\n");
}

#endif /* splay_h */