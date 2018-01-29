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


#include "sf_process_allocation.h"

// **
// This function determines on which core each selected process will run the next quantum
// **
void process_allocation (int all_p) {
    node *aux, *sel;
    
    int i;
    int occupied_core[MAX_CPUS];
    
    safety_checks_PA (all_p);                          // This function simply controls the the workload and cores available fit the requirements of the implented policies
    
    for (aux = running_queue.head; aux; aux = aux->sig) {
        aux->selected = 0;
    }
    
    // **
    // 0
    // Process are randomly allocated on the cores
    // **
    if (all_p == 0 || running_queue.N < 4) {        // TO EDIT: the second condition prevents the need on considering the scenarios with 1, 2, and 3 processes in the
                                                    // all_p = 1 and all_p = 2 process allocation policies.
                                                    // Obviously, this scenarios can be considered, but not doing so simplifies the lab session.
        
        // This vector identifies the cores where a process has already benn allocated
        for (i=0; i<MAX_CPUS; i++) {
            occupied_core[i] = 0;
        }
        
        for (aux = running_queue.head; aux; aux = aux->sig) {
            
            do {
                i = rand() % running_queue.N;
            } while (occupied_core[i]);
            
            aux->cores[0] = available_cores[i];     // Asing the selected core to the list of available cores for that process
            aux->n_cores = 1;                       // The process is only allowed to run in a single core
            occupied_core[i] = 1;                   // Set the core as already assigned
        }
    }
    
    
    // **
    // L1-bandwidth aware process allocation
    // Balances the L1 requests among the cores
    // Only supports four applications and two SMT cores
    // TO EDIT: it can be removed and ask its developement to students
    // **
    else if (all_p == 1) {
        
        
        // **
        //
        // LAB5: IMPLEMENT DE L1 BANDWIDTH-AWARE PROCESS ALLOCATION POLICY HERE
        //
        // **
        
        
    }
    
    
    // **
    // L1-bandwidth aware process allocation
    // Unbalances the L1 requests among the cores
    // **
    else if (all_p == 2) {
        
        
        // **
        //
            // LAB5: IMPLEMENT DE BAD L1 BANDWIDTH-AWARE PROCESS ALLOCATION POLICY HERE
        //
        // **
    
    
    }
    
    else {
        fprintf(stderr, "ERROR!! Process selection policy (%d) not available.\n", all_p);
        exit (0);
    }
    
    // This function effectively allocates the processes on their assigned core
    allocate_jobs_to_cores();
}


// **
// This function sets the cpu_set_t masks and binds the execution of the processes on the selected cores
// **
void allocate_jobs_to_cores () {
    cpu_set_t mask;
    node *aux;
    int i;
    
    for (aux = running_queue.head; aux; aux = aux->sig) {
        CPU_ZERO(&mask);
        for (i = 0; i < aux->n_cores; i++) {
            //fprintf(stderr, "Process %d_%d al core %d\n", aux->benchmark, aux->id, aux->cores[i]);
            CPU_SET(aux->cores[i], &mask);
        }
        
        if (sched_setaffinity(aux->pid, sizeof (mask), &mask) != 0) {
            fprintf(stderr, "Sched_setaffinity error: %d.\n", errno);
            exit(1);
        }
    }    
}


// **
// This function finds the node with the highest L1 bandwidth utilization
// **
node * find_max_BW_L1 () {
    node *aux;
    node *max_n = NULL;
    double max_v = -1;          // It should be below the L1 bandwidth utilization of any application
    
    for (aux = running_queue.head; aux; aux = aux->sig) {
        if (aux->selected == 0 && aux->BW_L1 > max_v) {
            max_n = aux;
            max_v = aux->BW_L1;
        }
    }
    max_n->selected = 1;
    return max_n;
}


// **
// This function finds the node with the lowest L1 bandwidth utilization
// **
node * find_min_BW_L1 () {
    node *aux;
    node *min_n = NULL;
    double min_v = 5000;    // It should be above the L1 bandwidth utilization of any application
    
    for (aux = running_queue.head; aux; aux = aux->sig) {
        if (aux->selected == 0 && aux->BW_L1 < min_v) {
            min_n = aux;
            min_v = aux->BW_L1;
        }
    }
    min_n->selected = 1;
    return min_n;
}


// **
// This function assigns the application of a node to a core of the indicated array of available cores
// **
void assign_node_to_core (node *n, int core) {
    
    n->cores[0] = available_cores [core];
    n->n_cores = 1;
}


// **
// This function assigns the remaining applications (not yet assigned to a core) of the running_queue to the second core
// To simplify the lab, its made to work with two cores and four apps. Thus, the remaining two apps are assigned to cores 2 and 3 of the available cores array
// **
void assign_remaining_applications () {
    int i = 2;
    node *aux;
    
    for (aux = running_queue.head; aux; aux = aux->sig) {
        
        if (aux->selected == 0) {
            
            if (i == 4) {           // Safety check. The condition should not be met.
                fprintf(stderr, "Error. This policy only performs process allocation for four applications in two SMT cores. You should extend it to support more scenarios\n");
                exit (-1);
            }
            
            aux->selected = 1;
            assign_node_to_core (aux, i);
            i++;
        }
    }
}


// **
// This function simply controls the the workload and cores available fit the requirements of the implented policies
// TO EDIT if the policies are extended to support other scnearios
// **
void safety_checks_PA (int all_p) {
    
    if (all_p == 1 || all_p == 2) {
        if (running_queue.N > 4) {
            fprintf(stderr, "Error. The implementation of the process allocation policy %d supports workloads of up to 4 applications.\n", all_p);
            exit (-1);
        }
        if (num_cores != 4) {
            fprintf(stderr, "Error. The implementation of the process allocation policy %d only supports scenarios with 4 logical CPUs.\n", all_p);
            exit (-1);
        }
    }
}












