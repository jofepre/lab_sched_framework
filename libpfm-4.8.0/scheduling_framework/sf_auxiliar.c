/*
 * Copyright (c) 2017 Universitat Politecnica de Valencia
 * Contributed by Josue Feliu <jofepre@gap.upv.es>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "sf_auxiliar.h"

// **
// This function initializes the node structure
// **
void initialize_node (node *n, int identificador, int benchmark, unsigned long long int target_insts) {
    int i;
    
    n->id = identificador;
    n->benchmark = benchmark;
    n->selected = 0;
    n->exec_quanta = 0;
    n->target_insts = target_insts;
    n->instructions = 0;
    n->cycles = 0;
    n->hits_L1 = 0;
    n->misses_L1 = 0;
    n->cores[0] = 0;
    n->BW_MM = 0;
    n->BW_L1 = 0;
    n->quantum_IPC = 0;
    
    for (i=0; i<MAX_EVENTS; i++) {
        n->events [i] = 0;
        n->events_q [i] = 0;
    }
    
    n->fds = NULL;
    n->sig = NULL;
    n->prv = NULL;
}


// **
// This function allocates the memory for a node and initializes it with ident and benchmark
// **
node *create_node (int ident, int benchmark, unsigned long long int target_insts) {
    
    node *aux;
    
    // Allocates the memory for a node
    aux = (node *) malloc (sizeof(node));
    if (aux == NULL) {
        fprintf(stderr, "Error allocating the memory for the node structure.\n");
        exit (-1);
    }
    
    initialize_node (aux, ident, benchmark, target_insts);
    
    return aux;
}


// **
// Insert the node n in the queue Q
// **
void insert_node (queue *Q, node *n) {
    if (!Q->N) { // Empty queue
        Q->head = n;
        Q->tail = n;
        Q->N = 1;
        n->prv = NULL;
        n->sig = NULL;
    }
    else {  // Not empty queue
        n->prv = Q->tail;
        n->sig = NULL;
        Q->tail->sig = n;
        Q->tail = n;
        Q->N++;
    }
}


// **
// Pull the node n from the queue Q
// **
void pull_node (queue *Q, node *n) {
    node *aux;
    
    if (Q->N == 1 && Q->head->id == n->id) {   // It is the only node of the queue
        Q->head = NULL;
        Q->tail = NULL;
    }
    
    else if (n->prv == NULL && Q->head->id == n->id) {  // It is the first node of the queue
        Q->head = n->sig;
        Q->head->prv = NULL;
    }
    
    else if (n->sig == NULL && Q->tail->id == n->id) {  // It is the last node of the queue
        Q->tail = n->prv;
        Q->tail->sig = NULL;
    }
    
    else {  // It should be in the middle of the queue
        for (aux = Q->head; aux; aux = aux->sig) {
            if (aux->id == n->id) {
                n->sig->prv = n->prv;
                n->prv->sig = n->sig;
                break;
            }
        }
    }
    Q->N--;
}


// **
// This function initializes the queue structure
// **
void initialize_queue (queue *Q) {
    Q->head = NULL;
    Q->tail = NULL;
    Q->N = 0;
}


// **
// This function lists the processes of a queue printing their information
// Note that is should be edited to make the printed content relevant to the lab session being developed
// **
void print_queue (queue *Q) {
    node *aux;
    fprintf(stderr, "Number of applications in the queue: %d.\n", Q->N);
    for (aux = Q->head; aux; aux = aux->sig) {
        
        // **
        // Example 1 for a lab sessions study the memory hierarchy
        // fprintf(stderr, "Application %d_%d -- Executed quanta %d -- Quantum_IPC: BW_MM: %f BW_L1: %f\n ", aux->benchmark, aux->id, aux->exec_quanta, aux->BW_MM, aux->BW_L1);
        // **
        
        fprintf(stderr, "Application %d_%d -- Executed quanta %d -- Quantum_IPC: %.3f\n", aux->benchmark, aux->id, aux->exec_quanta, aux->quantum_IPC);
    }
    fprintf(stderr, "\n");
}



