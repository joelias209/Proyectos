#ifndef avl_h
#define avl_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#define YES 1
#define NO 0

int max(int a, int b){
    return a > b ? a : b;
}

struct avl_node {
    int elem;
    struct avl_node *right;
    struct avl_node *left;
    int height;
};

typedef struct avl_node avl_tree;

int node_height(avl_tree *n){
    if(n == NULL)
        return -1;
    if(n->right == NULL && n->left == NULL)
        return 0;
    if(n->right == NULL && n->left != NULL)
        return n->left->height+1;
    if(n->right != NULL && n->left == NULL)
        return n->right->height+1;
    else
        return max(n->left->height, n->right->height)+1;
}

avl_tree * insert_avl(avl_tree *n, int e){
    if( n == NULL ){
        avl_tree *newTree = malloc(sizeof(avl_tree));
        if(newTree == NULL){
            printf("El elemento no se pudo insertar\n");
            return NULL;
        }
        newTree->elem = e;
        newTree->left = NULL;
        newTree->right = NULL;
        newTree->height = 0;
        return newTree;
    }
    if( n->elem == e )
        return n;

    if( e > n->elem ){ // insert a la derecha
        n->right = insert_avl(n->right, e);
        int height_right;
        int height_left;

        if( n->right != NULL )
            height_right = n->right->height;
        else
            height_right = -1;

        if(n->left == NULL)
            height_left = -1;
        else
            height_left = n->left->height;

        if( height_right-height_left > 1 ){
            if(e > n->right->elem){
                // RR y se arregla con una rotacion sencilla
                avl_tree *k2 = n->right;
                n->right = n->right->left;
                k2->left = n;
                n->height = node_height(n);
                k2->height = node_height(k2);
                return k2;
            }
            else{
                // RL y se arregla con doble rotación
                avl_tree *k1 = n;
                avl_tree *k2 = n->right->left;
                avl_tree *k3 = n->right;
                k1->right = k3->left->left;
                k3->left = k3->left->right;
                k2->right = k3;
                k2->left = k1;
                k1->height = node_height(k1);
                k3->height = node_height(k3);
                k2->height = node_height(k2);
                return k2;
            }
        }

        n->height = node_height(n);
        return n;

    } // insert a la derecha

    // insert a la izquierda
    n->left = insert_avl(n->left, e);
    int height_right;
    int height_left;

    if( n->right != NULL )
        height_right = n->right->height;
    else
        height_right = -1;

    if(n->left == NULL)
        height_left = -1;
    else
        height_left = n->left->height;

    if( height_left-height_right > 1 ){
        if(e < n->left->elem){
            // LL y se arregla con una rotacion sencilla
            avl_tree *k2 = n->left;
            n->left = n->left->right;
            k2->right = n;
            n->height = node_height(n);
            k2->height = node_height(k2);
            return k2;
        }
        else{
            // LR y se arregla con doble rotación
            avl_tree *k1 = n;
            avl_tree *k2 = n->left->right;
            avl_tree *k3 = n->left;
            k1->left = k3->right->right;
            k3->right = k3->right->left;
            k2->left = k3;
            k2->right = k1;
            k1->height = node_height(k1);
            k3->height = node_height(k3);
            k2->height = node_height(k2);
            return k2;
        }
    }

    n->height = node_height(n);
    return n;
}

int find_avl(avl_tree *n, int e){
    if(n == NULL)
        return NO;
    if( n->elem == e )
        return YES;
    if( e > n->elem )
        return find_avl(n->right, e);
    return find_avl(n->left, e);
}

void remove_avl(avl_tree **n, int e){
    if(*n == NULL)
        return;
    if( (*n)->elem == e ){
        if((*n)->right == NULL && (*n)->left == NULL){
            free(*n);
            *n = NULL;
            return;
        }
        if((*n)->right == NULL && (*n)->left != NULL){
            avl_tree *aux = *n;
            *n = (*n)->left;
            free(aux);
            return;
        }
        if((*n)->right != NULL && (*n)->left == NULL){
            avl_tree *aux = *n;
            *n = (*n)->right;
            free(aux);
            return;
        }
        if((*n)->right != NULL && (*n)->left != NULL){
            if((*n)->left->height > (*n)->right->height){
                avl_tree *aux = (*n)->left;
                while(aux->right != NULL)
                    aux = aux->right;
                (*n)->elem = aux->elem;
                remove_avl(&(*n)->left, aux->elem);
                (*n)->height = node_height(*n);
                return;
            }
            avl_tree *aux = (*n)->right;
            while(aux->left != NULL)
                aux = aux->left;
            (*n)->elem = aux->elem;
            remove_avl(&(*n)->right, aux->elem);
            (*n)->height = node_height(*n);
            return;
        }
    }
    if( e > (*n)->elem ){        
        remove_avl(&(*n)->right, e);
        int height_right;
        int height_left;

        if( (*n)->right != NULL )
            height_right = (*n)->right->height;
        else
            height_right = -1;

        if((*n)->left == NULL)
            height_left = -1;
        else
            height_left = (*n)->left->height;

        if( height_left - height_right > 1 ){
            int height_left_left;
            int height_left_right;

            if( (*n)->left->right != NULL )
                height_left_right = (*n)->left->right->height;
            else
                height_left_right = -1;

            if((*n)->left->left == NULL)
                height_left_left = -1;
            else
                height_left_left = (*n)->left->left->height;

            if(height_left_left > height_left_right){
                // LL y se arregla con una rotacion sencilla
                avl_tree *k2 = (*n)->left;
                (*n)->left = (*n)->left->right;
                k2->right = (*n);
                (*n)->height = node_height(*n);
                k2->height = node_height(k2);
                return;
            }
            else{
                // LR y se arregla con doble rotación
                avl_tree *k1 = *n;
                avl_tree *k2 = (*n)->left->right;
                avl_tree *k3 = (*n)->left;
                k1->left = k3->right->right;
                k3->right = k3->right->left;
                k2->left = k3;
                k2->right = k1;
                k1->height = node_height(k1);
                k3->height = node_height(k3);
                k2->height = node_height(k2);
                return;
            }
        }

        (*n)->height = node_height(*n);
        return;
    }
    remove_avl(&(*n)->left, e);
    int height_right;
    int height_left;

    if( (*n)->right != NULL )
        height_right = (*n)->right->height;
    else
        height_right = -1;

    if((*n)->left == NULL)
        height_left = -1;
    else
        height_left = (*n)->left->height;

    if( height_right - height_left > 1 ){
        int height_right_right;
        int height_right_left;

        if( (*n)->right->right != NULL )
            height_right_right = (*n)->right->right->height;
        else
            height_right_right = -1;

        if((*n)->right->left == NULL)
            height_right_left = -1;
        else
            height_right_left = (*n)->right->left->height;

        if(height_right_right > height_right_left){
            // RR y se arregla con una rotacion sencilla
            avl_tree *k2 = (*n)->right;
            (*n)->right = (*n)->right->left;
            k2->left = *n;
            (*n)->height = node_height(*n);
            k2->height = node_height(k2);
            return;
        }
        else{
            // RL y se arregla con doble rotación
            avl_tree *k1 = *n;
            avl_tree *k2 = (*n)->right->left;
            avl_tree *k3 = (*n)->right;
            k1->right = k3->left->left;
            k3->left = k3->left->right;
            k2->right = k3;
            k2->left = k1;
            k1->height = node_height(k1);
            k3->height = node_height(k3);
            k2->height = node_height(k2);
            return;
        }
    }

    (*n)->height = node_height(*n);
    return;
}

void free_avl(avl_tree *n){
    if(n == NULL)
        return;
    free_avl(n->left);
    free_avl(n->right);
    free(n);
}

void print_avl(avl_tree *n){
    if(n == NULL)
        return;
    print_avl(n->left);
    printf("%d ", n->elem);
    print_avl(n->right);
}

#endif /* avl_h */