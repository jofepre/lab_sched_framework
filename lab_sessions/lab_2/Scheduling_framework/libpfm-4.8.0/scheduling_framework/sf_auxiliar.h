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


#ifndef _SF_AUX_H_
#define _SF_AUX_H_

#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/types.h>
#include <inttypes.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <err.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <malloc.h>
#include <math.h>
#include <time.h>

#include "perf_util.h"


// **
//
// DEFINES
//
// **

#define MAX_CPUS 4
#define MAX_EVENTS 6            // Depends on the number of performance counters available
#define MAX_APPS 3
#define CPU_FREQ 2700


// **
//
// STRUCTS
//
// **

typedef struct _node {
    pid_t pid;                  // Proces PID
    int id;                     // Node identifier (useful when two instances of the same application are launched)
    int benchmark;              // Application (benchmark identifier)
    
    uint64_t target_insts;      // Target number of instructions (set in the node initialization to an offline proffiled value)
    
    int status;                 // Used as status variable in the waitpid functions
    
    perf_event_desc_t *fds;     // Descriptor to access performance counters
    
    int selected;               // Simple flag that determines when a process has already been selected in different parts of the code
    int cores [MAX_CPUS];       // Cores on which the process will be allowed to run in the next quantum
    int n_cores;                // Number of valid cores in the previous array
    
    uint64_t instructions;      // Overall number of instructions commited
    uint64_t instructions_q;    // Instructions commieted during the last quantum
    
    uint64_t cycles_q;          // Cycles running during the last quantum
    uint64_t cycles;            // Overall number of cycles running
    
    uint64_t events_q [MAX_EVENTS];          // Number of repetitions of event2 during the last quantum
    uint64_t events [MAX_EVENTS];            // Overall Number of repetitions of event2
    
    unsigned long hits_L1;      // L1 hits
    unsigned long misses_L1;    // L1 misses
    unsigned long LLC_misses;   // LLC misses
    
    int exec_quanta;            // Accounts for the quanta where the process has run
    int completion_quantum;     // Quantum on which the application reaches its target instructions
    
    double BW_L1;               // Bandwidth to access the L1 cache
    double BW_MM;               // Bandwidth to access main memory
    
    double IPC;                 // Saves the IPC of the application once it is completed
    double quantum_IPC;         // Saves the IPC of the application during the las quantum
    
    struct _node * sig;         // Pointer to the next node in the queue
    struct _node * prv;         // Pointer to the previous node in the queue
} node;


typedef struct _queue {
    node * head;                // Pointer to the head node in the queue
    node * tail;                // Pointer to the tail node in the queue (not used)
    int N;                      // Number of nodes in the queue
} queue;


typedef struct {
    char *events;
    int delay;
    int pinned;
    int group;
} options_t;


// **
//
// VARIABLES
//
// **

int quantum;                    // Global quantum counter (accounts the execution quanta)

int num_cores;                  // Input parameter. Number of logical cores considered in the experiment.

int print_per_quantum;          // Determines if the event counts should be printed every quantum

queue process_queue;            // Queue of available processes
queue running_queue;            // Queue of processes selected to run the next quantum
queue finished_queue;           // Queue of finished processes

options_t options;              // Options struct variable
int num_descriptors;            // Number of events being monitored

int *available_cores;           // Array containing the available cores where processes can run (it can be set through an input parameter)

int workload [MAX_APPS];        // Array containing the applications of the workload to be run
                                // TODO: avoid the maximum number of applications to depend from the MAX_APPS define

char *event_names [MAX_EVENTS]; // Save the name of the events being monitored

struct timeval tvv;             // Auxiliary structures to measure the execution time and scheduling overhead
struct timezone tzz;
double start_time_sch, end_time_sch, start_time_aux, end_time_aux;
double total_scheduling, total_aux;

double BW_L1_medio;
double BW_MM_medio;


// **
//
// FUNCTIONS
//
// **

// **
// This function initializes the node structure
// **
void initialize_node (node *n, int identificador, int benchmark, unsigned long long int target_insts);

// **
// This function allocates the memory for a node and initializes it with ident and benchmark
// **
node *create_node (int ident, int benchmark, unsigned long long int target_insts);

// **
// Insert the node n in the queue Q
// **
void insert_node (queue *Q, node *n);

// **
// Pull the node n from the queue Q
// **
void pull_node (queue *Q, node *n);

// **
// This function initializes the queue structure
// **
void initialize_queue (queue *Q);

// **
// This function lists the processes of a queue printing their information
// Note that is should be edited to make the printed content relevant to the lab session being developed
// **
void print_queue (queue *Q);

#endif


