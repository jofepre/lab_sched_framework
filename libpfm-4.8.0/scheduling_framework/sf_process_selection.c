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
        
        
        //fprintf(stderr, "Quantum %d\n", quantum);
        
        bw_remain = calculate_avg_MM_BW_per_quantum();          // Set BW remain to the average bandwidth utilization per quantum
        
        if (quantum > 5 && bw_remain == 0) {                    // Safety check
            fprintf(stderr, "Error. BW remain is not being updated.\n");
            exit (-1);
        }
        
        cpu_remain = num_cores;                         // Set cpu_remain to the number of availabel cpus, unless the number of processes is below this number
        if (process_queue.N < cpu_remain) {
            cpu_remain = process_queue.N;
        }
        
        //fprintf(stderr, "BW_remain: %.3f\n", bw_remain);
        //fprintf(stderr, "Cpu_reamin: %d\n", cpu_remain);
        
        
        // The first process of the process_queue is always selected to avoid starvation
        //fprintf(stderr, "First selected process: %d_%d\n", process_queue.head->benchmark, process_queue.head->id);
        aux = process_queue.head;
        bw_remain -= aux->BW_MM;
        cpu_remain --;
        pull_node (&process_queue, aux);
        insert_node (&running_queue, aux);
        
        //fprintf(stderr, "Bw_remain: %.3f - Cpu remain %d\n", bw_remain, cpu_remain);
        
        
        // The remaining processes are selected according to their fit to the remaining bandwdith per core
        while (cpu_remain) {
            
            max_fitness = -1;
            max_node = process_queue.head;                              // Avoid max_node = NULL in the firs quanta
            for (aux = process_queue.head; aux; aux = aux->sig) {
                fit = fitness(bw_remain, aux->BW_MM, cpu_remain);      // We look for the higher fit
                if (fit > max_fitness) {
                    max_fitness = fit;
                    max_node = aux;
                }
            }
            
            // Insert the selected node in the running_queue and update bw_remain and cpu_remain
            //fprintf(stderr, "Process %d_%d selected\n", max_node->benchmark, max_node->id);
            pull_node (&process_queue, max_node);
            bw_remain -= max_node->BW_MM;
            cpu_remain--;
            insert_node (&running_queue, max_node);
        }
        
        
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
























