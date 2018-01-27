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


#include "sf_process_selection.h"

// **
// This function determines which processes will run the next quantum
// **
void process_selection (int sel_p) {
    node *aux, *sig, *max_node;
    int cpu_remain, n, i;
    double bw_remain, max_fitness, fit;
    
    // **
    // Select all the processes to be run the next quantum
    // **
    if (sel_p == 0) {
        for (aux = process_queue.head; aux; aux = sig) {
            pull_node (&process_queue, aux);
            sig = aux->sig;
            insert_node (&running_queue, aux);
        }
    }
    
    
    // **
    // Randomly select as many processes as available number of cores
    // **
    else if (sel_p == 1) {
        
        cpu_remain = num_cores;                 // num_cores refers to logical CPUs (an SMT2 core consists of two logical CPUs for Linux)
        if (process_queue.N < cpu_remain) {
            cpu_remain = process_queue.N;
        }
        
        for (cpu_remain; cpu_remain; cpu_remain --) {
            aux = process_queue.head;
            n = rand() % process_queue.N;  // random number between 0 and N-1
            for (i=0; i<n; i++) {
                aux = aux->sig;
            }
            
            pull_node (&process_queue, aux);
            insert_node (&running_queue, aux);
        }
    }
    
    
    // **
    // Main memory bandwidth-aware process selection
    // Selects each quantum as many processes as available number of cores balancing the bandwidth consumption over the workload execution time
    // The BW_MM variable of the nodes should be correctly updated to use this policy
    // **
    else if (sel_p == 2) {
        
        
        // **
        //
        // LAB4: IMPLEMENT A MAIN MEMORY BANDWIDTH-AWARE PROCESS SELECTION POLICY HERE
        //
        // **
        
        
    }
    
    else {
        fprintf(stderr, "ERROR!! Process selection policy (%d) not available.\n", sel_p);
        exit (-1);
    }
    
    //fprintf(stderr, "\nSELECTED PROCESSES: \n");
    //print_queue (&running_queue);
}


// **
// This function calculates the fit between predicted and remain per cpu
// The closer remain is to predicted/cpus the higher the fit
// **
double fitness (double remain, double predicted, double cpus) {
    double aux;
    
    aux = (remain/cpus) - predicted;
    if (aux < 0) {
        aux = -aux;
    }
    return 1/aux;
}


// **
// This function calculates the average bandwidth utilization per quantum
// **
double calculate_avg_MM_BW_per_quantum() {
    node *aux;
    double bw = 0;
    
    for (aux = process_queue.head; aux; aux = aux->sig) {
        bw += aux->BW_MM;
    }
    
    return bw  / ((double)process_queue.N / num_cores);
}
























